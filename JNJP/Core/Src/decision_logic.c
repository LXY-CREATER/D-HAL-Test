#include "decision_logic.h"
#include "vehicle_detect.h"
#include "speed_calc.h"
#include "traffic_light.h"
#include "display.h"

static uint32_t lastVehicleTime1 = 0;
static uint32_t lastVehicleTime2 = 0;
static uint8_t screenOff1 = 0;
static uint8_t screenOff2 = 0;

void DecisionLogic_Init(void)
{
    lastVehicleTime1 = 0;
    lastVehicleTime2 = 0;
    screenOff1 = 0;
    screenOff2 = 0;
}

void DecisionLogic_Process(void)
{
    uint8_t hasVehicle1 = (g_lane1Status.vehicleType != VEHICLE_NONE);
    uint8_t hasVehicle2 = (g_lane2Status.vehicleType != VEHICLE_NONE);
    uint8_t hasOncoming = hasVehicle1 && hasVehicle2;
    
    if (hasVehicle1) {
        lastVehicleTime1 = HAL_GetTick();
        screenOff1 = 0;
        Display_SetBacklight(DISPLAY_LANE1, 1);
    }
    
    if (hasVehicle2) {
        lastVehicleTime2 = HAL_GetTick();
        screenOff2 = 0;
        Display_SetBacklight(DISPLAY_LANE2, 1);
    }
    
    if (!hasVehicle1 && !hasVehicle2) {
        if (HAL_GetTick() - lastVehicleTime1 > SCREEN_TIMEOUT_MS && !screenOff1) {
            Display_SetBacklight(DISPLAY_LANE1, 0);
            screenOff1 = 1;
        }
        if (HAL_GetTick() - lastVehicleTime2 > SCREEN_TIMEOUT_MS && !screenOff2) {
            Display_SetBacklight(DISPLAY_LANE2, 0);
            screenOff2 = 1;
        }
    }
    
    if (hasVehicle1 || hasVehicle2) {
        g_decisionResult.safeSpeed1 = SpeedCalc_Calculate(g_lane1Status.vehicleType, hasOncoming);
        g_decisionResult.safeSpeed2 = SpeedCalc_Calculate(g_lane2Status.vehicleType, hasOncoming);
    } else {
        g_decisionResult.safeSpeed1 = 0;
        g_decisionResult.safeSpeed2 = 0;
    }
    
    if (!hasVehicle1 && !hasVehicle2) {
        g_decisionResult.alertLevel1 = ALERT_LEVEL_SAFE;
        g_decisionResult.alertLevel2 = ALERT_LEVEL_SAFE;
        g_decisionResult.ledColor1 = LED_COLOR_GREEN;
        g_decisionResult.ledColor2 = LED_COLOR_GREEN;
    } else if (g_lane1Status.vehicleType == VEHICLE_LARGE && g_lane2Status.vehicleType == VEHICLE_LARGE) {
        g_decisionResult.alertLevel1 = ALERT_LEVEL_DANGER;
        g_decisionResult.alertLevel2 = ALERT_LEVEL_DANGER;
        g_decisionResult.ledColor1 = LED_COLOR_RED;
        g_decisionResult.ledColor2 = LED_COLOR_RED;
    } else if (hasOncoming && (g_lane1Status.vehicleType == VEHICLE_LARGE || g_lane2Status.vehicleType == VEHICLE_LARGE)) {
        g_decisionResult.alertLevel1 = ALERT_LEVEL_DANGER;
        g_decisionResult.alertLevel2 = ALERT_LEVEL_DANGER;
        g_decisionResult.ledColor1 = LED_COLOR_RED;
        g_decisionResult.ledColor2 = LED_COLOR_RED;
    } else if (hasOncoming) {
        g_decisionResult.alertLevel1 = ALERT_LEVEL_WARNING;
        g_decisionResult.alertLevel2 = ALERT_LEVEL_WARNING;
        g_decisionResult.ledColor1 = LED_COLOR_YELLOW;
        g_decisionResult.ledColor2 = LED_COLOR_YELLOW;
    } else if (g_lane1Status.vehicleType == VEHICLE_LARGE || g_lane2Status.vehicleType == VEHICLE_LARGE) {
        if (g_lane1Status.vehicleType == VEHICLE_LARGE) {
            g_decisionResult.alertLevel1 = ALERT_LEVEL_WARNING;
            g_decisionResult.ledColor1 = LED_COLOR_YELLOW;
        } else {
            g_decisionResult.alertLevel1 = ALERT_LEVEL_NORMAL;
            g_decisionResult.ledColor1 = LED_COLOR_GREEN;
        }
        
        if (g_lane2Status.vehicleType == VEHICLE_LARGE) {
            g_decisionResult.alertLevel2 = ALERT_LEVEL_WARNING;
            g_decisionResult.ledColor2 = LED_COLOR_YELLOW;
        } else {
            g_decisionResult.alertLevel2 = ALERT_LEVEL_NORMAL;
            g_decisionResult.ledColor2 = LED_COLOR_GREEN;
        }
    } else {
        g_decisionResult.alertLevel1 = ALERT_LEVEL_NORMAL;
        g_decisionResult.alertLevel2 = ALERT_LEVEL_NORMAL;
        g_decisionResult.ledColor1 = LED_COLOR_GREEN;
        g_decisionResult.ledColor2 = LED_COLOR_GREEN;
    }
    
    TrafficLight_SetAll(g_decisionResult.ledColor1, g_decisionResult.ledColor2);
    
    // 移除对Display_ShowSpeed和Display_ShowIdle函数的调用

}

DecisionResult_t* DecisionLogic_GetResult(void)
{
    return &g_decisionResult;
}
