#ifndef __SPEED_CALC_H
#define __SPEED_CALC_H

#include "main.h"
#include "config.h"

void SpeedCalc_Init(void);
uint8_t SpeedCalc_Calculate(VehicleType_t vehicleType, uint8_t hasOncomingVehicle);
uint8_t SpeedCalc_GetSafeSpeed(LaneStatus_t *lane1, LaneStatus_t *lane2);

#endif
