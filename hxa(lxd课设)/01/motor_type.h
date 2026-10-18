#ifndef __MOTOR_TYPE_H
#define __MOTOR_TYPE_H

#include "main.h"
#include "tim.h"

// 电机控制引脚定义
#ifndef AIN1_Pin
#define AIN1_Pin GPIO_PIN_4
#define AIN1_GPIO_Port GPIOB
#endif

#ifndef AIN2_Pin
#define AIN2_Pin GPIO_PIN_5
#define AIN2_GPIO_Port GPIOB
#endif

extern uint8_t bt_param_mode;
extern uint8_t bt_last_cmd;
extern uint32_t bt_param_timeout;

// 方便使用的PWM设置宏
#define Moto_PWM1_SetCompare(value) \
    do { \
        if(htim3.State == HAL_TIM_STATE_READY) \
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); \
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (value)); \
    } while(0)

#define Moto_PWM2_SetCompare(value) \
    do { \
        if(htim3.State == HAL_TIM_STATE_READY) \
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); \
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, (value)); \
    } while(0)

// 电机操作宏（直接操作GPIO，用于调试）
#define MOTOR_FORWARD_GPIO() \
    do { \
        HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET); \
        HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET); \
    } while(0)

#define MOTOR_BACKWARD_GPIO() \
    do { \
        HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET); \
        HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET); \
    } while(0)

#define MOTOR_STOP_GPIO() \
    do { \
        HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET); \
        HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET); \
    } while(0)

#define MOTOR_BRAKE_GPIO() \
    do { \
        HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET); \
        HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET); \
    } while(0)

// 函数声明
void Moto_Fast_PWMForword(uint8_t speed);
void Moto_Slow_PWMForword(uint8_t speed);
void Moto_Fast_PWMReversal(uint8_t speed);
void Moto_Slow_PWMReversal(uint8_t speed);
void Moto_Stop(void);
void Moto_Brake(void);

// 增强的电机控制函数
void Motor_Forward(uint8_t speed);      // 正转
void Motor_Backward(uint8_t speed);     // 反转
void Motor_Stop(void);                  // 停止
void Motor_Control(int8_t speed);       // 带方向的速度控制(-100到100)
void Motor_Set_Speed(uint8_t speed);    // 设置速度（0-100%）
uint8_t Motor_Get_Speed(void);          // 获取当前速度
void Motor_Emergency_Stop(void);        // 紧急停止
void Motor_Test_Routine(void);          // 测试例程

// 编码器相关函数
void Encoder_Update(void);              // 更新编码器数据
int32_t Encoder_Get_Position(void);     // 获取编码器位置
int16_t Encoder_Get_Velocity(void);     // 获取编码器速度
void Encoder_Reset(void);               // 重置编码器
float Encoder_Get_RPM(void);            // 获取电机转速(RPM)

// 阻力模式相关函数
void Resistance_Mode_Init(void);        // 阻力模式初始化
void Resistance_Mode_Update(void);      // 阻力模式更新
void Set_Resistance_Level(uint8_t level); // 设置阻力等级(0-10)
uint8_t Get_Resistance_Level(void);     // 获取阻力等级

// 系统函数
void System_Init(void);                 // 系统初始化
void Main_Task(void);                   // 主任务
void Send_System_Status(void);          // 发送系统状态
void Safety_Check(void);                // 安全检查
void Update_Exercise_Count(void);       // 更新锻炼计数
void Send_Help_Info(void);
void Check_Bluetooth_Param_Timeout(void);

// 蓝牙处理函数
void Process_Bluetooth_Command(uint8_t cmd);  // 处理蓝牙命令

#endif
