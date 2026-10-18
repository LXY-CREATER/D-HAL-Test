#ifndef SERVO_H
#define SERVO_H

#include "tim.h"

// 定义舵机通道枚举
typedef enum {
    SERVO_HORIZONTAL = TIM_CHANNEL_3,  // 水平舵机 (PD14)
    SERVO_VERTICAL   = TIM_CHANNEL_4   // 垂直舵机 (PD15)
} Servo_Channel;

// 舵机控制函数
void Servo_Init(void);
void Servo_SetAngle(Servo_Channel channel, float angle);
void Servo_SetPosition(Servo_Channel channel, float angle);

#endif
