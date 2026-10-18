#ifndef CONTROL_CONTROL_H_
#define CONTROL_CONTROL_H_

#include "stm32f1xx_hal.h" //HAL库文件声明
#include <main.h>
#include "motor.h"

extern TIM_HandleTypeDef htim2;//声明TIM2的HAL库结构体

void CAR_GO();                 //小车前进
void CAR_BACK();               //小车后退
void CAR_LGO();                //小车原地左拐
void CAR_RGO();                //小车原地右拐
void CAR_STOP();               //小车停止

#endif /* CONTROL_CONTROL_H_ */
