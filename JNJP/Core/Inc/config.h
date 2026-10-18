#ifndef __CONFIG_H
#define __CONFIG_H

#include "main.h"

#define CURVE_RADIUS_DEFAULT        50.0f
#define FRICTION_COEFF_DEFAULT      0.7f
#define GRAVITY                     9.8f

#define PRESSURE_THRESHOLD_NONE     300
#define PRESSURE_THRESHOLD_SMALL    1200
#define PRESSURE_THRESHOLD_LARGE    2500

#define MAX_SPEED_LARGE_KMH         30
#define MAX_SPEED_SMALL_KMH         40
#define MIN_SPEED_KMH               10

#define FILTER_WINDOW_SIZE          10
#define DETECT_STABLE_COUNT         5

#define SCREEN_TIMEOUT_MS           30000
#define DISPLAY_REFRESH_MS          100
#define SENSOR_SAMPLE_MS            10

#define TASK_PRIORITY_HIGH          4
#define TASK_PRIORITY_NORMAL        3
#define TASK_PRIORITY_LOW           2

#define TASK_STACK_SIZE_SMALL       256
#define TASK_STACK_SIZE_NORMAL      512
#define TASK_STACK_SIZE_LARGE       1024

typedef enum {
    VEHICLE_NONE = 0,
    VEHICLE_SMALL,
    VEHICLE_LARGE
} VehicleType_t;

typedef enum {
    ALERT_LEVEL_SAFE = 0,
    ALERT_LEVEL_NORMAL,
    ALERT_LEVEL_WARNING,
    ALERT_LEVEL_DANGER
} AlertLevel_t;

typedef enum {
    LED_COLOR_GREEN = 0,
    LED_COLOR_YELLOW,
    LED_COLOR_RED,
    LED_COLOR_OFF
} LedColor_t;

typedef struct {
    float curveRadius;
    float frictionCoeff;
    uint16_t thresholdNone;
    uint16_t thresholdSmall;
    uint16_t thresholdLarge;
    uint8_t maxSpeedLarge;
    uint8_t maxSpeedSmall;
} SystemConfig_t;

typedef struct {
    VehicleType_t vehicleType;
    uint16_t pressureValue;
    uint8_t isStable;
    uint32_t lastDetectTime;
} LaneStatus_t;

typedef struct {
    uint8_t safeSpeed1;
    uint8_t safeSpeed2;
    AlertLevel_t alertLevel1;
    AlertLevel_t alertLevel2;
    LedColor_t ledColor1;
    LedColor_t ledColor2;
} DecisionResult_t;

extern SystemConfig_t g_sysConfig;
extern LaneStatus_t g_lane1Status;
extern LaneStatus_t g_lane2Status;
extern DecisionResult_t g_decisionResult;

void Config_Init(void);
void Config_SetDefault(void);

#endif
