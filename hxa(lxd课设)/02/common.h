#ifndef __COMMON_H
#define __COMMON_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

// ============================================
// 统一的结构体定义
// ============================================

// 编码器数据类型
typedef struct {
    TIM_HandleTypeDef* timer;          // 编码器定时器
    int32_t count;                     // 累计计数
    int32_t raw_count;                 // 原始计数
    float position_mm;                 // 当前位置(mm)
    float velocity_mm_s;               // 当前速度(mm/s)
    uint32_t last_update_time;         // 最后更新时间
    uint8_t reverse;                   // 方向反转标志
    uint8_t initialized;               // 初始化标志
    float counts_per_mm;               // 每毫米计数
} Encoder_Type;

// 电机状态类型
typedef struct {
    uint8_t direction;                 // 方向
    uint8_t speed_percent;             // 速度百分比
    uint16_t pwm_duty;                 // PWM占空比值
    float position_mm;                 // 当前位置(mm)
    float position_percent;            // 当前位置百分比
    float target_position_mm;          // 目标位置(mm)
    float target_position_percent;     // 目标位置百分比
    uint8_t is_moving;                 // 是否在运动
    uint8_t emergency_stop_flag;       // 紧急停止标志
    uint8_t calibrated;                // 是否已校准
    float min_limit_mm;                // 最小限制位置(mm)
    float max_limit_mm;                // 最大限制位置(mm)
    float travel_range_mm;             // 行程范围(mm)
    uint32_t last_update_time;         // 最后更新时间
    uint8_t soft_limit_enabled;        // 软限位使能
    uint8_t auto_stop_enabled;         // 自动停止使能
} MotorStatus_Type;

// 电机配置类型
typedef struct {
    TIM_HandleTypeDef* pwm_tim;        // PWM定时器
    uint32_t pwm_channel_1;            // PWM通道1
    uint32_t pwm_channel_2;            // PWM通道2
    uint16_t pwm_max_value;            // PWM最大值
    uint16_t pwm_min_value;            // PWM最小值
    uint16_t pwm_max_limit;            // PWM最大限制
} MotorConfig_Type;

// 康复参数类型
typedef struct {
    uint8_t resistance_level;          // 阻力等级
    uint8_t is_active;                 // 是否激活
    uint8_t auto_adjust;               // 自动调整
    float target_range_mm;             // 目标范围
    float current_range_mm;            // 当前范围
    float position_min_mm;             // 最小位置
    float position_max_mm;             // 最大位置
    float center_position_mm;          // 中心位置
    float target_velocity;             // 目标速度
    float current_velocity;            // 当前速度
    uint32_t last_update_time;         // 最后更新时间
    uint32_t exercise_count;           // 运动计数
    float total_distance_mm;           // 总距离
} RehabilitationParams_Type;

// 蓝牙状态类型
typedef struct {
    uint8_t rx_buffer[128];            // 接收缓冲区
    uint8_t tx_buffer[128];            // 发送缓冲区
    uint8_t cmd_queue[20][16];         // 命令队列
    uint16_t rx_index;                 // 接收索引
    uint16_t cmd_head;                 // 命令队列头
    uint16_t cmd_tail;                 // 命令队列尾
    uint8_t is_connected;              // 连接状态
    uint32_t last_rx_time;             // 最后接收时间
    uint32_t error_count;              // 错误计数
    uint8_t dma_active;                // DMA激活标志
} BluetoothStatus_Type;

// 命令结构体
typedef struct {
    uint8_t command;
    uint8_t parameter;
    uint32_t timestamp;
} BluetoothCmd_Type;

#endif /* __COMMON_H */
