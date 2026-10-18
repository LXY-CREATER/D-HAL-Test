#ifndef __CONFIG_H
#define __CONFIG_H

#include "main.h"

// 系统时钟
#define SYSTEM_CLOCK_FREQ    72000000  // 72MHz
#define TICK_PERIOD_MS       1         // 系统定时器周期

// 电机参数
#define MOTOR_MAX_SPEED      100       // 最大速度（PWM占空比）
#define MOTOR_MIN_SPEED      10        // 最小速度
#define MOTOR_STOP_SPEED     0         // 停止速度
#define ENCODER_MAX_COUNT    65535     // 编码器最大计数值
#define ENCODER_RESOLUTION   12        // 编码器分辨率

// 阻力模式参数
#define RESISTANCE_FACTOR    0.5f      // 阻力系数
#define MAX_RESISTANCE       50        // 最大阻力
#define MIN_RESISTANCE       5         // 最小阻力
#define DEAD_ZONE            5         // 死区

// 串口配置
#define UART_BUFFER_SIZE     64
#define UART_TIMEOUT_MS      1000

// OLED配置
#define OLED_ADDRESS         0x78      // OLED I2C地址
#define OLED_WIDTH           128
#define OLED_HEIGHT          64
#define OLED_PAGE_SIZE       8

// 蓝牙命令定义
#define CMD_FORWARD          'F'
#define CMD_BACKWARD         'B'
#define CMD_STOP             'S'
#define CMD_LEFT             'L'
#define CMD_RIGHT            'R'
#define CMD_SET_SPEED        'V'
#define CMD_SET_MODE         'M'
#define CMD_GET_STATUS       'G'
#define CMD_SET_RESISTANCE   'R'
#define CMD_CALIBRATE        'C'

// 语音命令代码（根据ASRPRO设置）
#define VOICE_CMD_FORWARD    0x01
#define VOICE_CMD_BACKWARD   0x02
#define VOICE_CMD_STOP       0x03
#define VOICE_CMD_SPEED_UP   0x04
#define VOICE_CMD_SPEED_DOWN 0x05
#define VOICE_CMD_MODE_CHANGE 0x06
#define VOICE_CMD_LEFT       0x07
#define VOICE_CMD_RIGHT      0x08

// 错误代码定义
#define ERROR_NONE              0
#define ERROR_EMERGENCY_STOP    1    // 添加紧急停止错误
#define ERROR_OVERHEAT          2
#define ERROR_MOTOR_STALL       3
#define ERROR_LOW_BATTERY       4
#define ERROR_MOTOR_OVERCURRENT 5
#define ERROR_ENCODER_FAILURE   6

// 工作模式定义
typedef enum {
    MODE_BLUETOOTH = 0,     // 蓝牙控制模式
    MODE_VOICE,             // 语音控制模式
    MODE_REHABILITATION,    // 康复阻力模式
    MODE_TEACH_PLAYBACK,    // 示教回放模式
    MODE_SETTINGS,          // 设置模式
    MODE_CALIBRATION        // 校准模式
} SystemMode_t;

// 电机状态定义
typedef enum {
    MOTOR_STOPPED = 0,
    MOTOR_FORWARD,
    MOTOR_BACKWARD,
    MOTOR_BRAKING,
    MOTOR_RESISTANCE        // 添加阻力模式状态
} MotorState_t;

// 错误代码枚举
typedef enum {
    ERROR_CODE_NONE = 0,
    ERROR_CODE_MOTOR_OVERCURRENT,
    ERROR_CODE_ENCODER_FAILURE,
    ERROR_CODE_COMMUNICATION,
    ERROR_CODE_OVERHEAT,
    ERROR_CODE_LOW_BATTERY,
    ERROR_CODE_EMERGENCY_STOP    // 添加紧急停止
} ErrorCode_t;

// 系统状态结构体
typedef struct {
    SystemMode_t mode;               // 工作模式
    MotorState_t motor_state;        // 电机状态
    uint8_t motor_speed;             // 当前速度(0-100%)
    int16_t encoder_position;        // 编码器位置
    int16_t encoder_speed;           // 编码器速度(计数/秒)
    uint8_t resistance_level;        // 阻力等级(0-10)
    uint8_t battery_level;           // 电池电量(0-100%)
    uint8_t temperature;             // 温度(℃)
    ErrorCode_t error_code;          // 错误代码
    uint32_t runtime_ms;             // 运行时间(ms)
    uint16_t exercise_count;         // 锻炼次数（康复模式下使用）
    uint16_t exercise_goal;          // 锻炼目标（康复模式下使用）
    uint8_t emergency_stop;          // 紧急停止标志
} SystemStatus_t;

// 全局系统状态变量声明（在motor_type.c中定义）
extern SystemStatus_t system_status;

#endif
