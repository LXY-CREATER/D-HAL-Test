#ifndef __APP_GLOBALS_H
#define __APP_GLOBALS_H

#include "config.h"

// ============================================
// 全局状态结构体
// ============================================

// 编码器状态
typedef struct {
    TIM_HandleTypeDef* timer;
    int32_t count;
    int32_t raw_count;
    float position_mm;
    float velocity_mm_s;
    uint32_t last_update_time;
    uint8_t reverse;
    uint8_t initialized;
    float counts_per_mm;
} EncoderState;

// 电机状态
typedef struct {
    MotorDirection direction;
    uint8_t speed_percent;
    uint16_t pwm_duty;
    float position_mm;
    float position_percent;
    float target_position_mm;
    float target_position_percent;
    uint8_t is_moving;
    uint8_t emergency_stop;
    uint8_t calibrated;
    float min_limit_mm;
    float max_limit_mm;
    float travel_range_mm;
    uint32_t last_update_time;
    uint8_t soft_limit_enabled;
    uint8_t auto_stop_enabled;
} MotorState;

// 电机配置
typedef struct {
    TIM_HandleTypeDef* pwm_tim;
    uint32_t pwm_channel_1;
    uint32_t pwm_channel_2;
    uint16_t pwm_period;
    uint16_t pwm_min_duty;
    uint16_t pwm_max_duty;
} MotorConfig;

// 蓝牙状态
typedef struct {
    uint8_t rx_buffer[BT_RX_BUFFER_SIZE];
    uint8_t tx_buffer[BT_TX_BUFFER_SIZE];
    BluetoothCmd cmd_queue[BT_CMD_QUEUE_SIZE];
    uint8_t rx_index;
    uint8_t cmd_head;
    uint8_t cmd_tail;
    uint8_t is_connected;
    uint32_t last_rx_time;
    uint32_t error_count;
    uint8_t dma_active;
} BluetoothState;

// 康复参数
typedef struct {
    uint8_t resistance_level;
    uint8_t is_active;
    uint8_t auto_adjust;
    float target_range_mm;
    float current_range_mm;
    float position_min_mm;
    float position_max_mm;
    float center_position_mm;
    float target_velocity;
    float current_velocity;
    uint32_t last_update_time;
    uint32_t exercise_count;
    float total_distance_mm;
} RehabState;

// ============================================
// 全局变量访问函数声明
// ============================================

// 编码器
EncoderState* get_encoder_state(void);
void set_encoder_state(EncoderState* state);

// 电机
MotorState* get_motor_state(void);
MotorConfig* get_motor_config(void);

// 蓝牙
BluetoothState* get_bluetooth_state(void);

// 康复
RehabState* get_rehab_state(void);

// 系统
void set_system_state(uint8_t state);
uint8_t get_system_state(void);
void set_system_mode(uint8_t mode);
uint8_t get_system_mode(void);
uint8_t is_system_ready(void);
void set_system_time(uint32_t time);
uint32_t get_system_time(void);

#endif /* __APP_GLOBALS_H */
