#ifndef __GLOBAL_VARS_H
#define __GLOBAL_VARS_H

#include "common.h"  // ʹ��ͳһ�Ľṹ�嶨��

// ============================================
// ������״̬����
// ============================================

typedef struct {
    uint8_t rx_buffer[64];
    uint8_t tx_buffer[64];
    uint8_t rx_index;
    uint8_t connected;
    uint32_t last_activity;
    char last_command[16];
    uint8_t new_command;
} VoiceState_t;

typedef struct {
    uint8_t current_page;
    uint8_t display_mode;
    uint32_t last_update;
    uint8_t need_refresh;
    uint8_t is_initialized;
    uint8_t brightness;
} OLEDStatus_t;

typedef struct {
    uint8_t state;
    uint8_t mode;
    uint8_t is_ready;
    uint8_t debug_enabled;
    uint32_t system_time;
    uint32_t last_update;
} SystemStatus_t;

typedef struct {
    uint8_t rx_buffer[128];
    uint8_t tx_buffer[128];
    uint8_t cmd_queue[20][16];
    uint16_t rx_index;
    uint16_t cmd_head;
    uint16_t cmd_tail;
    uint8_t is_connected;
    uint32_t last_rx_time;
    uint32_t error_count;
    uint8_t dma_active;
} BluetoothStatus_t;

typedef struct {
    uint8_t last_command;
    uint32_t last_command_time;
    uint8_t is_ready;
    uint8_t error_count;
} VoiceStatus_t;

typedef struct {
    int32_t count;
    int32_t raw_count;
    float position_mm;
    float velocity_mm_s;
    uint32_t last_update_time;
    uint8_t reverse;
    uint8_t initialized;
    float counts_per_mm;
    TIM_HandleTypeDef* timer;
} EncoderData_t;

#endif /* __GLOBAL_VARS_H */
