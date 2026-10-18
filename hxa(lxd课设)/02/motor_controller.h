#ifndef __MOTOR_CONTROLLER_H
#define __MOTOR_CONTROLLER_H

#include "config.h"
#include "app_globals.h"
// ============================================
// ???????
// ============================================
typedef struct {
    MotorDirection direction;      // ???????
    uint8_t speed_percent;         // ??????(0-100%)
    uint16_t pwm_duty;             // PWM?????
    float position_mm;             // ???¦Ë??(mm)
    float position_percent;        // ???¦Ë?e???(0-100%)
    float target_position_mm;      // ???¦Ë??(mm)
    float target_position_percent; // ???¦Ë?e???
    uint8_t is_moving;             // ????????
    uint8_t emergency_stop;        // ?????????
    uint8_t calibrated;            // ?????§µ?
    float min_limit_mm;            // ??§³????¦Ë??(mm)
    float max_limit_mm;            // ???????¦Ë??(mm)
    float travel_range_mm;         // ?§Ô??¦¶(mm)
    uint32_t last_update_time;     // ?????????
    uint8_t soft_limit_enabled;    // ????¦Ë???
    uint8_t auto_stop_enabled;     // ????????
} MotorStatus;

// ============================================
// ??????y?
// ============================================
typedef struct {
    TIM_HandleTypeDef* pwm_tim;    // PWM?????
    uint32_t pwm_channel_1;        // PWM???1
    uint32_t pwm_channel_2;        // PWM???2
    uint16_t pwm_period;           // PWM?????
    uint16_t pwm_min_duty;         // ??§³PWM????
    uint16_t pwm_max_duty;         // ???PWM????
} MotorConfig;

// ============================================
// ????????
// ============================================

// AT8870????????
void AT8870_Set_Output(uint8_t in1, uint8_t in2);
void AT8870_Safe_Stop(void);

// ?????????
void Motor_Init(TIM_HandleTypeDef* pwm_tim, uint32_t ch1, uint32_t ch2);
void Motor_Deinit(void);

// ???????????
void Motor_Set_Direction(MotorDirection dir);
void Motor_Set_Speed(uint8_t percent);
void Motor_Stop(void);
void Motor_Brake(void);
void Motor_Emergency_Brake(void);
void Motor_Force_Stop(void);
void Motor_Smooth_Stop(void);

// ??????????
void Motor_Go_Forward(uint8_t speed);
void Motor_Go_Backward(uint8_t speed);
void Motor_Move_To_Position_MM(float position_mm, uint8_t speed);
void Motor_Move_To_Percent(float percent, uint8_t speed);
void Motor_Move_To_Center(uint8_t speed);

// ¦Ë?¨´???????
void Motor_Set_Position_Limits(float min_mm, float max_mm);
void Motor_Enable_Soft_Limit(uint8_t enable);
void Motor_Enable_Auto_Stop(uint8_t enable);
uint8_t Motor_Check_Position_Safety(float target_mm);
void Motor_Set_Zero_Position(void);
void Motor_Calibrate(void);
void Motor_Reset_Emergency_Stop(void);

// ?????o???
void Motor_Update_Position(float current_mm);
void Motor_Update_Target_Check(void);

// ?????????
MotorDirection Motor_Get_Direction(void);
uint8_t Motor_Get_Speed(void);
float Motor_Get_Position_MM(void);
float Motor_Get_Position_Percent(void);
uint8_t Motor_Is_Moving(void);
uint8_t Motor_Is_At_Limit(void);
uint8_t Motor_Is_Emergency_Stop(void);

// ???????
void Motor_Test_Routine(void);
void Motor_Check_Pin_State(void);

#endif /* __MOTOR_CONTROLLER_H */
