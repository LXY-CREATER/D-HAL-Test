#include "vehicle_detect.h"
#include "sensor.h"

static uint8_t detectCounter1 = 0;
static uint8_t detectCounter2 = 0;
static VehicleType_t lastType1 = VEHICLE_NONE;
static VehicleType_t lastType2 = VEHICLE_NONE;

void VehicleDetect_Init(void)
{
    detectCounter1 = 0;
    detectCounter2 = 0;
    lastType1 = VEHICLE_NONE;
    lastType2 = VEHICLE_NONE;
}

VehicleType_t VehicleDetect_Classify(uint16_t pressure)
{
    if (pressure < g_sysConfig.thresholdNone) {
        return VEHICLE_NONE;
    } else if (pressure < g_sysConfig.thresholdSmall) {
        return VEHICLE_SMALL;
    } else {
        return VEHICLE_LARGE;
    }
}

void VehicleDetect_Update(void)
{
    VehicleType_t currentType;
    
    currentType = VehicleDetect_Classify(g_lane1Status.pressureValue);
    if (currentType == lastType1) {
        if (detectCounter1 < DETECT_STABLE_COUNT) {
            detectCounter1++;
        }
        if (detectCounter1 >= DETECT_STABLE_COUNT) {
            g_lane1Status.vehicleType = currentType;
            g_lane1Status.isStable = 1;
        }
    } else {
        detectCounter1 = 0;
        lastType1 = currentType;
        g_lane1Status.isStable = 0;
    }
    
    currentType = VehicleDetect_Classify(g_lane2Status.pressureValue);
    if (currentType == lastType2) {
        if (detectCounter2 < DETECT_STABLE_COUNT) {
            detectCounter2++;
        }
        if (detectCounter2 >= DETECT_STABLE_COUNT) {
            g_lane2Status.vehicleType = currentType;
            g_lane2Status.isStable = 1;
        }
    } else {
        detectCounter2 = 0;
        lastType2 = currentType;
        g_lane2Status.isStable = 0;
    }
}

void VehicleDetect_Process(void)
{
    Sensor_ReadAll();
    VehicleDetect_Update();
}
