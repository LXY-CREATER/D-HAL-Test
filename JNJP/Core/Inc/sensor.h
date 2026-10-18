#ifndef __SENSOR_H
#define __SENSOR_H

#include "main.h"
#include "config.h"
#include "adc.h"

#define SENSOR_LANE1    0
#define SENSOR_LANE2    1

#define PEAK_HOLD_TIME_MS       5000    // 检测到压力后保持5秒
#define PEAK_DECAY_STEP         10      // 衰减步长
#define MIN_PEAK_TRIGGER        20      // 超过基准值20就触发峰值检测
#define BASELINE_SAMPLES        10      // 基准值采样次数

typedef struct {
    uint16_t buffer[FILTER_WINDOW_SIZE];
    uint8_t index;
    uint8_t count;
    uint16_t sum;
    uint16_t filteredValue;
    uint16_t rawValue;
    uint16_t peakValue;          // 检测到的峰值
    uint32_t peakTimestamp;      // 峰值检测时间
    uint8_t peakDetected;        // 是否检测到有效峰值
    uint16_t baseline;           // 动态基准值
    uint8_t baselineCount;       // 基准值计数器
} SensorFilter_t;

void Sensor_Init(void);
void Sensor_Start(void);
void Sensor_Update(void);
uint16_t Sensor_GetRawValue(uint8_t lane);
uint16_t Sensor_GetFilteredValue(uint8_t lane);
uint16_t Sensor_GetPeakValue(uint8_t lane);
uint16_t Sensor_GetBaseline(uint8_t lane);
void Sensor_UploadData(void);
void Sensor_ReadAll(void);

#endif
