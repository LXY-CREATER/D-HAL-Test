#include "app_globals.h"
#include <string.h>

// ============================================
// 静态全局变量定义
// ============================================

static EncoderState encoder_state = {
    .timer = NULL,
    .count = 0,
    .raw_count = 0,
    .position_mm = 0,
    .velocity_mm_s = 0,
    .last_update_time = 0,
    .reverse = 0,
    .initialized = 0,
    .counts_per_mm = COUNTS_PER_MM
};

static MotorState motor_state = {
    .direction = MOTOR_STOP,
    .speed_percent = 0,
    .pwm_duty = 0,
    .position_mm = 0,
    .position_percent = 0,
    .target_position_mm = 0,
    .target_position_percent = 0,
    .is_moving = 0,
    .emergency_stop = 0,
    .calibrated = 0,
    .min_limit_mm = SAFETY_MARGIN_MM,
    .max_limit_mm = TRAVEL_RANGE_MM - SAFETY_MARGIN_MM,
    .travel_range_mm = TRAVEL_RANGE_MM - 2 * SAFETY_MARGIN_MM,
    .last_update_time = 0,
    .soft_limit_enabled = 1,
    .auto_stop_enabled = 1
};

static MotorConfig motor_config = {
    .pwm_tim = NULL,
    .pwm_channel_1 = 0,
    .pwm_channel_2 = 0,
    .pwm_period = PWM_PERIOD,
    .pwm_min_duty = MIN_PWM_DUTY,
    .pwm_max_duty = MAX_PWM_DUTY
};

static BluetoothState bt_state = {
    .rx_index = 0,
    .cmd_head = 0,
    .cmd_tail = 0,
    .is_connected = 0,
    .last_rx_time = 0,
    .error_count = 0,
    .dma_active = 0
};

static RehabState rehab_state = {
    .resistance_level = REHAB_DEFAULT_LEVEL,
    .is_active = 0,
    .auto_adjust = 1,
    .target_range_mm = REHAB_TARGET_RANGE_MM,
    .current_range_mm = 0,
    .position_min_mm = 50.0f,
    .position_max_mm = 150.0f,
    .center_position_mm = 100.0f,
    .target_velocity = 30.0f,
    .current_velocity = 0,
    .last_update_time = 0,
    .exercise_count = 0,
    .total_distance_mm = 0
};

// 系统变量
static uint8_t system_state = SYS_STATE_INIT;
static uint8_t system_mode = MODE_IDLE;
static uint8_t system_ready = 0;
static uint32_t system_time = 0;

// ============================================
// 全局变量访问函数
// ============================================

EncoderState* get_encoder_state(void) {
    return &encoder_state;
}

void set_encoder_state(EncoderState* state) {
    if (state != NULL) {
        encoder_state = *state;
    }
}

MotorState* get_motor_state(void) {
    return &motor_state;
}

MotorConfig* get_motor_config(void) {
    return &motor_config;
}

BluetoothState* get_bluetooth_state(void) {
    return &bt_state;
}

RehabState* get_rehab_state(void) {
    return &rehab_state;
}

void set_system_state(uint8_t state) {
    system_state = state;
}

uint8_t get_system_state(void) {
    return system_state;
}

void set_system_mode(uint8_t mode) {
    system_mode = mode;
}

uint8_t get_system_mode(void) {
    return system_mode;
}

uint8_t is_system_ready(void) {
    return system_ready;
}

void set_system_time(uint32_t time) {
    system_time = time;
}

uint32_t get_system_time(void) {
    return system_time;
}
