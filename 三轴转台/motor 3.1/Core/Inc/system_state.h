#ifndef __SYSTEM_STATE_H
#define __SYSTEM_STATE_H

#include "stm32f1xx_hal.h"

// 系统状态定义
typedef enum {
    SYS_STATE_INIT,
		SYS_STATE_ENABLING_MOTORS,
    SYS_STATE_RUNNING,
    SYS_STATE_EMERGENCY_STOP
} SystemState;

// 系统状态机数据结构
typedef struct {
    SystemState current_state;
    uint32_t enable_timeout;
    uint32_t last_heartbeat;
    uint8_t motor_enabled[3]; // 0:X, 1:Y, 2:Z
} SystemStateMachine;

// 声明全局变量
extern SystemState g_system_state;

// 函数声明
void system_state_machine(void);
void emergency_button_handler(void);

#endif
