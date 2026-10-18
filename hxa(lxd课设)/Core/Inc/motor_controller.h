/* USER CODE BEGIN Header */

	/**
  ******************************************************************************
  * @file           : motor_controller.h
  * @brief          : Motor controller header file
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __MOTOR_CONTROLLER_H
#define __MOTOR_CONTROLLER_H

#include "main.h"
#include "tim.h"
#include "config.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

// ������PWM��������
#ifndef MOTOR_PWM_MAX_DUTY
#define MOTOR_PWM_MAX_DUTY     1000  // ������PWM��������
#endif
#ifndef MOTOR_PWM_MIN_DUTY
#define MOTOR_PWM_MIN_DUTY     0     // ������PWM��������
#endif

// ������������������
#ifndef TRAVEL_RANGE_MM
#define TRAVEL_RANGE_MM        200.0f  // ������200mm
#endif
#ifndef PHYSICAL_TRAVEL_RANGE_MM
#define PHYSICAL_TRAVEL_RANGE_MM 205.0f  // ������205mm
#endif
#ifndef SAFETY_MARGIN_MM
#define SAFETY_MARGIN_MM       5.0f    // ������5mm
#endif

/* USER CODE END Private defines */

/* USER CODE BEGIN Private typedefs */

// ������
typedef enum {
    MOTOR_FORWARD = 0,
    MOTOR_BACKWARD,
    MOTOR_STOP,
    MOTOR_BRAKE,
    MOTOR_ERROR
} MotorDirection_t;

// 方向常量定义
#define MOTOR_DIR_STOP      MOTOR_STOP
#define MOTOR_DIR_FORWARD   MOTOR_FORWARD
#define MOTOR_DIR_BACKWARD  MOTOR_BACKWARD

// 电机配置
typedef struct {
    TIM_HandleTypeDef* pwm_tim;
    uint32_t pwm_channel_1;    // PWM通道1(用于正转)
    uint32_t pwm_channel_2;    // PWM通道2(用于反转)
    uint16_t pwm_max_value;
    uint16_t pwm_min_value;
    uint16_t pwm_max_limit;
    GPIO_TypeDef* pb5_port;    // PB5端口
    uint16_t pb5_pin;          // PB5引脚
} MotorConfig_t;

// ��������
typedef struct {
    MotorDirection_t direction;
    uint8_t speed_percent;
    uint16_t pwm_duty;
    float position_mm;
    float travel_range_mm;
    float min_limit_mm;
    float max_limit_mm;
    uint8_t soft_limit_enabled;
    uint8_t emergency_stop_flag;
    uint32_t last_update;
} MotorStatus_t;

/* USER CODE END Private typedefs */

/* USER CODE BEGIN Private variables */

// ��������
extern MotorStatus_t g_motor_status;
// ��������
extern MotorConfig_t g_motor_config;

/* USER CODE END Private variables */

/* USER CODE BEGIN Private function prototypes */

// 电机初始化
void Motor_Controller_Init(TIM_HandleTypeDef* pwm_tim, uint32_t ch1, uint32_t ch2, uint16_t pwm_period);

// ������
void Motor_Set_Direction(MotorDirection_t dir);

// ������
void Motor_Set_Speed(uint8_t speed_percent);

// ������
void Motor_Go_Forward(uint8_t speed_percent);

// ������
void Motor_Go_Backward(uint8_t speed_percent);

// ������
void Motor_Stop(void);

// ������
void Motor_Brake(void);

// �������
void Motor_Set_PWM(uint16_t duty_ch1, uint16_t duty_ch2);

// �������
void Motor_Update_Position(void);

// �������
uint8_t Motor_Check_Position_Safety(float target_mm);

// �������
void Motor_Set_Position_Limits(float min_mm, float max_mm);

// ����λ�õ���
void Motor_Update_Position_MM(float position_mm);

// ��ȡλ��
float Motor_Get_Position_MM(void);
float Motor_Get_Position_Percent(void);

// ��ȡ��ǰ�ٶ�
uint8_t Motor_Get_Speed(void);

// ��ȡ����
MotorDirection_t Motor_Get_Direction(void);

// ����ɲ��
void Motor_Emergency_Stop(void);
void Motor_Emergency_Brake(void);
void Motor_Force_Stop(void);
void Motor_Clear_Emergency_Stop(void);
void Motor_Reset_Emergency_Stop(void);

// ����״̬
uint8_t Motor_Is_Moving(void);

// ����λ��
void Motor_Set_Zero_Position(void);

// ����Ŀ�꣬
void Motor_Update_Target_Check(void);

// ��ȡ����״̬
uint8_t Motor_Is_Emergency_Stop(void);

// �ƶ���ָ����λ��
void Motor_Move_To_Percent(uint8_t percent, uint8_t speed);

// �������
void Motor_Test_Routine(void);

/* USER CODE END Private function prototypes */

#endif /* __MOTOR_CONTROLLER_H */
