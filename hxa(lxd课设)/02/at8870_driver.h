#ifndef __AT8870_DRIVER_H
#define __AT8870_DRIVER_H

#include "config.h"
#include "common.h"
// ============================================
// AT8870真值表
// ============================================
// IN1  IN2  OUT1  OUT2  Motor
//  0    0    Hi-Z  Hi-Z  停止/滑行
//  1    0    H     L     前进
//  0    1    L     H     后退
//  1    1    L     L     刹车

typedef enum {
    AT8870_STOP = 0,      // 00 - 停止/滑行
    AT8870_FORWARD,       // 10 - 前进
    AT8870_BACKWARD,      // 01 - 后退
    AT8870_BRAKE          // 11 - 刹车
} AT8870_State;

// ============================================
// 函数声明
// ============================================

// 初始化
void AT8870_Init(TIM_HandleTypeDef* pwm_tim, uint32_t ch1, uint32_t ch2);

// 基础控制
void AT8870_Set_State(AT8870_State state, uint16_t pwm_duty);
void AT8870_Stop(void);
void AT8870_Brake(void);
void AT8870_Coast(void);
void AT8870_Emergency_Stop(void);

// PWM控制
void AT8870_Set_PWM(uint16_t duty_in1, uint16_t duty_in2);
void AT8870_Stop_PWM(void);
void AT8870_Start_PWM(void);

// 状态获取
AT8870_State AT8870_Get_State(void);
uint8_t AT8870_Is_Running(void);

#endif /* __AT8870_DRIVER_H */
