#ifndef __SYSTEM_STATE_H
#define __SYSTEM_STATE_H

#include "stm32f1xx_hal.h"

// 系统状态定义
typedef enum {
    SYS_STATE_INIT,
    SYS_STATE_ENABLING_MOTORS,  // 新增：使能电机状态
    SYS_STATE_RUNNING,
    SYS_STATE_EMERGENCY_STOP

} SystemState;

// 系统状态机数据结构
typedef struct {
    SystemState current_state;
    uint32_t enable_timeout;
    uint32_t last_heartbeat;
		uint32_t last_status_print;    // 上次状态打印时间
    uint32_t last_enable_sent;     // 上次发送使能命令时间
    uint8_t enable_retry_count;    // 使能重试次数
    uint8_t motor_enabled[3]; // 0:X, 1:Y, 2:Z
} SystemStateMachine;

// 声明全局变量
extern SystemStateMachine g_system;

// 函数声明
void system_state_machine(void);
void emergency_button_handler(void);

#endif
