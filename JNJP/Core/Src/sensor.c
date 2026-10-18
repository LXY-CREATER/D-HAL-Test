#include "sensor.h"

static SensorFilter_t lane1Filter;
static SensorFilter_t lane2Filter;
static uint16_t adcValues[2];
static uint8_t adcConversionComplete = 0;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1) {
        adcConversionComplete = 1;
    }
}

void Sensor_Init(void)
{
    uint8_t i;
    
    for (i = 0; i < FILTER_WINDOW_SIZE; i++) {
        lane1Filter.buffer[i] = 0;
        lane2Filter.buffer[i] = 0;
    }
    
    lane1Filter.index = 0;
    lane1Filter.count = 0;
    lane1Filter.sum = 0;
    lane1Filter.filteredValue = 0;
    lane1Filter.rawValue = 0;
    lane1Filter.peakValue = 0;
    lane1Filter.peakTimestamp = 0;
    lane1Filter.peakDetected = 0;
    lane1Filter.baseline = 0;
    lane1Filter.baselineCount = 0;
    
    lane2Filter.index = 0;
    lane2Filter.count = 0;
    lane2Filter.sum = 0;
    lane2Filter.filteredValue = 0;
    lane2Filter.rawValue = 0;
    lane2Filter.peakValue = 0;
    lane2Filter.peakTimestamp = 0;
    lane2Filter.peakDetected = 0;
    lane2Filter.baseline = 0;
    lane2Filter.baselineCount = 0;
    
    adcValues[0] = 0;
    adcValues[1] = 0;
    adcConversionComplete = 0;
}

void Sensor_Start(void)
{
}

static void Sensor_UpdatePeak(SensorFilter_t *filter)
{
    uint32_t now = HAL_GetTick();
    
    if (filter->baselineCount < BASELINE_SAMPLES) {
        filter->baseline += filter->filteredValue;
        filter->baselineCount++;
        if (filter->baselineCount == BASELINE_SAMPLES) {
            filter->baseline /= BASELINE_SAMPLES;
        }
        return;
    }
    
    uint16_t triggerLevel = filter->baseline + MIN_PEAK_TRIGGER;
    
    if (filter->filteredValue > triggerLevel && filter->filteredValue > filter->peakValue) {
        filter->peakValue = filter->filteredValue;
        filter->peakTimestamp = now;
        filter->peakDetected = 1;
    }
    
    if (filter->peakDetected && (now - filter->peakTimestamp > PEAK_HOLD_TIME_MS)) {
        if (filter->peakValue > filter->baseline + PEAK_DECAY_STEP) {
            filter->peakValue -= PEAK_DECAY_STEP;
        } else {
            filter->peakValue = 0;
            filter->peakDetected = 0;
            filter->baseline = 0;
            filter->baselineCount = 0;
        }
    }
}

static uint16_t Sensor_Filter(SensorFilter_t *filter, uint16_t newValue)
{
    filter->sum -= filter->buffer[filter->index];
    filter->buffer[filter->index] = newValue;
    filter->sum += newValue;
    filter->index = (filter->index + 1) % FILTER_WINDOW_SIZE;
    
    if (filter->count < FILTER_WINDOW_SIZE) {
        filter->count++;
    }
    
    filter->filteredValue = filter->sum / filter->count;
    
    return filter->filteredValue;
}

void Sensor_Update(void)
{
    lane1Filter.rawValue = adcValues[0];
    lane2Filter.rawValue = adcValues[1];
    
    Sensor_Filter(&lane1Filter, adcValues[0]);
    Sensor_Filter(&lane2Filter, adcValues[1]);
    
    Sensor_UpdatePeak(&lane1Filter);
    Sensor_UpdatePeak(&lane2Filter);
    
    g_lane1Status.pressureValue = lane1Filter.peakDetected ? lane1Filter.peakValue : lane1Filter.filteredValue;
    g_lane2Status.pressureValue = lane2Filter.peakDetected ? lane2Filter.peakValue : lane2Filter.filteredValue;
}

uint16_t Sensor_GetRawValue(uint8_t lane)
{
    if (lane == SENSOR_LANE1) {
        return lane1Filter.rawValue;
    } else {
        return lane2Filter.rawValue;
    }
}

uint16_t Sensor_GetFilteredValue(uint8_t lane)
{
    if (lane == SENSOR_LANE1) {
        return lane1Filter.filteredValue;
    } else {
        return lane2Filter.filteredValue;
    }
}

uint16_t Sensor_GetBaseline(uint8_t lane)
{
    if (lane == SENSOR_LANE1) {
        return lane1Filter.baseline;
    } else {
        return lane2Filter.baseline;
    }
}

void Sensor_UploadData(void)
{
    UsartConfig_SendString("{\"device\":\"JNJP001\",");
    
    UsartConfig_SendString("\"lane1_raw\":");
    UsartConfig_SendNumber(Sensor_GetRawValue(0));
    UsartConfig_SendString(",\"lane1_peak\":");
    UsartConfig_SendNumber(Sensor_GetPeakValue(0));
    UsartConfig_SendString(",\"lane1_base\":");
    UsartConfig_SendNumber(Sensor_GetBaseline(0));
    UsartConfig_SendString(",\"lane1_diff\":");
    UsartConfig_SendNumber(Sensor_GetRawValue(0) > Sensor_GetBaseline(0) ? Sensor_GetRawValue(0) - Sensor_GetBaseline(0) : 0);
    
    UsartConfig_SendString(",\"lane2_raw\":");
    UsartConfig_SendNumber(Sensor_GetRawValue(1));
    UsartConfig_SendString(",\"lane2_peak\":");
    UsartConfig_SendNumber(Sensor_GetPeakValue(1));
    
    UsartConfig_SendString(",\"uptime\":");
    UsartConfig_SendNumber(HAL_GetTick() / 1000);
    UsartConfig_SendString("}\r\n");
}

uint16_t Sensor_GetPeakValue(uint8_t lane)
{
    if (lane == SENSOR_LANE1) {
        return lane1Filter.peakValue;
    } else {
        return lane2Filter.peakValue;
    }
}

void Sensor_ReadAll(void)
{
    HAL_ADC_Start(&hadc1);
    
    HAL_ADC_PollForConversion(&hadc1, 10);
    adcValues[0] = HAL_ADC_GetValue(&hadc1);
    
    HAL_ADC_PollForConversion(&hadc1, 10);
    adcValues[1] = HAL_ADC_GetValue(&hadc1);
    
    HAL_ADC_Stop(&hadc1);
    
    Sensor_Update();
}
