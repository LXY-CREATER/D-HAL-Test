#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "stm32f1xx_hal.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

//添加Π定义

#define M_PI 3.14159265358979323846f  // 单精度浮点π值


// 系统参数
#define SYS_CLOCK_FREQ     72000000  // 72MHz
#define CONTROL_FREQ       100       // 控制频率(Hz)
#define CONTROL_PERIOD_MS  10        // 控制周期(ms)

// CAN配置
#define CAN_MASTER_ID      0x201     // 主轴CAN ID
#define CAN_SLAVE1_ID      0x202     // 从轴1 CAN ID
#define CAN_SLAVE2_ID      0x203     // 从轴2 CAN ID
#define CAN_BAUDRATE       500000    // 500kbps

// 电机控制模式
#define MIT_MODE_POSITION  0x01      // 位置模式
#define MIT_MODE_VELOCITY  0x02      // 速度模式
#define MIT_MODE_CURRENT   0x03      // 电流模式

// 运动参数
#define MAX_ROLL_ANGLE     45.0f     // 最大横滚角(度)
#define MAX_PITCH_ANGLE    45.0f     // 最大俯仰角(度)
#define MAX_YAW_ANGLE      180.0f    // 最大偏航角(度)

// 安全参数
#define MAX_TEMP           80.0f     // 最大允许温度(℃)
#define MAX_CURRENT        5.0f      // 最大允许电流(A)

// 安全状态码
#define SAFETY_OK          0x00
#define SAFETY_ERROR_TEMP  0x01
#define SAFETY_ERROR_CURRENT 0x02
#define SAFETY_ERROR_POSITION 0x03
#define SAFETY_ERROR_EMERGENCY 0x04

// 电机数量
#define MOTOR_COUNT        3

// 系统状态枚举
typedef enum {
    SYS_IDLE = 0,
    SYS_RUNNING,
    SYS_EMERGENCY_STOP
} SystemState_t;

// 电机状态结构体
typedef struct {
    uint8_t id;             // 电机ID
    float position;         // 当前位置(度)
    float velocity;         // 当前速度(度/秒)
    float current;          // 当前电流(A)
    float temperature;      // 当前温度(℃)
    float target_position;  // 目标位置(度)
    float target_velocity;  // 目标速度(度/秒)
    float target_current;   // 目标电流(A)
    uint8_t mode;           // 控制模式
    uint8_t error;          // 错误状态
} MotorState_t;

// 全局变量声明
extern MotorState_t Motors[MOTOR_COUNT];
extern SystemState_t SystemState;
extern CAN_HandleTypeDef hcan;
extern UART_HandleTypeDef huart1;

// 函数声明
void SystemClock_Config(void);
void Error_Handler(void);

#endif /* __CONFIG_H__ */
