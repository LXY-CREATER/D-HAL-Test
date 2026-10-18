#ifndef __VEHICLE_DETECT_H
#define __VEHICLE_DETECT_H

#include "main.h"
#include "config.h"

void VehicleDetect_Init(void);
VehicleType_t VehicleDetect_Classify(uint16_t pressure);
void VehicleDetect_Update(void);
void VehicleDetect_Process(void);

#endif
