#ifndef __TRAFFIC_LIGHT_H
#define __TRAFFIC_LIGHT_H

#include "main.h"
#include "config.h"

#define TRAFFIC_LIGHT_LANE1    0
#define TRAFFIC_LIGHT_LANE2    1

void TrafficLight_Init(void);
void TrafficLight_SetColor(uint8_t laneId, LedColor_t color);
void TrafficLight_SetAll(LedColor_t color1, LedColor_t color2);
void TrafficLight_Off(uint8_t laneId);
void TrafficLight_AllOff(void);

#endif
