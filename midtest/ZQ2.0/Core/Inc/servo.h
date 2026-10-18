#ifndef __SERVO_H
#define __SERVO_H

#include "main.h"

// 初始化舵机（TIM句柄和通道）
void Servo_Init(TIM_HandleTypeDef *htim, uint32_t channel);
// 设置舵机角度（0-180度）
void Servo_SetAngle(uint8_t angle);

#endif
