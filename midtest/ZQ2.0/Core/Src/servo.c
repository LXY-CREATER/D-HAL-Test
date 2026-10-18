#include "servo.h"

static TIM_HandleTypeDef *servoTim;
static uint32_t servoChannel;

// 初始化舵机（使用TIM1_CH1，PE9，20ms周期）
void Servo_Init(TIM_HandleTypeDef *htim, uint32_t channel) {
    servoTim = htim;
    servoChannel = channel;
    HAL_TIM_PWM_Start(servoTim, servoChannel);  // 启动PWM
}

// 设置角度（0-180度对应PWM脉宽500-2500us）
void Servo_SetAngle(uint8_t angle) {
    if (angle > 180) angle = 180;
    uint32_t pulse = 500 + (angle * 2000) / 180;  // 计算脉宽
    __HAL_TIM_SET_COMPARE(servoTim, servoChannel, pulse);
}
