#include "speed_calc.h"
#include <math.h>

void SpeedCalc_Init(void)
{
}

uint8_t SpeedCalc_Calculate(VehicleType_t vehicleType, uint8_t hasOncomingVehicle)
{
    float baseSpeed;
    float safeSpeed;
    float modifier = 1.0f;
    
    baseSpeed = sqrtf(g_sysConfig.frictionCoeff * GRAVITY * g_sysConfig.curveRadius);
    baseSpeed *= 3.6f;
    
    if (vehicleType == VEHICLE_LARGE) {
        modifier *= 0.75f;
    }
    
    if (hasOncomingVehicle) {
        modifier *= 0.85f;
    }
    
    safeSpeed = baseSpeed * modifier;
    
    if (vehicleType == VEHICLE_LARGE) {
        if (safeSpeed > g_sysConfig.maxSpeedLarge) {
            safeSpeed = g_sysConfig.maxSpeedLarge;
        }
    } else if (vehicleType == VEHICLE_SMALL) {
        if (safeSpeed > g_sysConfig.maxSpeedSmall) {
            safeSpeed = g_sysConfig.maxSpeedSmall;
        }
    }
    
    if (safeSpeed < MIN_SPEED_KMH) {
        safeSpeed = MIN_SPEED_KMH;
    }
    
    return (uint8_t)safeSpeed;
}

uint8_t SpeedCalc_GetSafeSpeed(LaneStatus_t *lane1, LaneStatus_t *lane2)
{
    uint8_t hasOncoming = 0;
    VehicleType_t primaryType = VEHICLE_NONE;
    
    if (lane1->vehicleType != VEHICLE_NONE && lane2->vehicleType != VEHICLE_NONE) {
        hasOncoming = 1;
        primaryType = (lane1->vehicleType > lane2->vehicleType) ? 
                       lane1->vehicleType : lane2->vehicleType;
    } else if (lane1->vehicleType != VEHICLE_NONE) {
        primaryType = lane1->vehicleType;
    } else if (lane2->vehicleType != VEHICLE_NONE) {
        primaryType = lane2->vehicleType;
    } else {
        return 0;
    }
    
    return SpeedCalc_Calculate(primaryType, hasOncoming);
}
