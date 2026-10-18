#include "config.h"

SystemConfig_t g_sysConfig;
LaneStatus_t g_lane1Status;
LaneStatus_t g_lane2Status;
DecisionResult_t g_decisionResult;

void Config_Init(void)
{
    Config_SetDefault();
    
    g_lane1Status.vehicleType = VEHICLE_NONE;
    g_lane1Status.pressureValue = 0;
    g_lane1Status.isStable = 0;
    g_lane1Status.lastDetectTime = 0;
    
    g_lane2Status.vehicleType = VEHICLE_NONE;
    g_lane2Status.pressureValue = 0;
    g_lane2Status.isStable = 0;
    g_lane2Status.lastDetectTime = 0;
    
    g_decisionResult.safeSpeed1 = 0;
    g_decisionResult.safeSpeed2 = 0;
    g_decisionResult.alertLevel1 = ALERT_LEVEL_SAFE;
    g_decisionResult.alertLevel2 = ALERT_LEVEL_SAFE;
    g_decisionResult.ledColor1 = LED_COLOR_GREEN;
    g_decisionResult.ledColor2 = LED_COLOR_GREEN;
}

void Config_SetDefault(void)
{
    g_sysConfig.curveRadius = CURVE_RADIUS_DEFAULT;
    g_sysConfig.frictionCoeff = FRICTION_COEFF_DEFAULT;
    g_sysConfig.thresholdNone = PRESSURE_THRESHOLD_NONE;
    g_sysConfig.thresholdSmall = PRESSURE_THRESHOLD_SMALL;
    g_sysConfig.thresholdLarge = PRESSURE_THRESHOLD_LARGE;
    g_sysConfig.maxSpeedLarge = MAX_SPEED_LARGE_KMH;
    g_sysConfig.maxSpeedSmall = MAX_SPEED_SMALL_KMH;
}
