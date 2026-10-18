#ifndef __ENCODER_H
#define __ENCODER_H

#include "config.h"
#include "app_globals.h"

// ============================================
// 函数声明
// ============================================

// 初始化函数
void Encoder_Init(TIM_HandleTypeDef* timer);
void Encoder_Deinit(void);
void Encoder_Init_Ex(TIM_HandleTypeDef* timer, uint16_t cpr, float gear_ratio, float pulley_circumference_mm);

// 更新函数
void Encoder_Update(void);

// 获取数据函数
int32_t Encoder_Get_Count(void);
int32_t Encoder_Get_Raw_Count(void);
float Encoder_Get_Position_MM(void);
float Encoder_Get_Position_Percent(void);
float Encoder_Get_Velocity_MM_S(void);
int32_t Encoder_Get_Velocity(void);
int32_t Encoder_Get_Delta(void);

// 设置/重置函数
void Encoder_Set_Reverse(uint8_t reverse);
void Encoder_Reset_Position_MM(float position_mm);
void Encoder_Reset(void);
void Encoder_Reset_To_Zero(void);

// 转换函数
float Encoder_Counts_To_MM(int32_t counts);
int32_t Encoder_MM_To_Counts(float mm);
int32_t Encoder_Percent_To_Counts(float percent);

// 状态检查函数
uint8_t Encoder_Is_Moving(void);
uint8_t Encoder_Is_At_Limit(void);
uint8_t Encoder_Is_Initialized(void);

// 校准函数
void Encoder_Direction_Test(void);
void Encoder_Calibrate(void);

#endif /* __ENCODER_H */
