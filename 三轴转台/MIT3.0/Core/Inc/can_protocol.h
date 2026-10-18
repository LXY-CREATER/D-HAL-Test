#ifndef __CAN_PROTOCOL_H
#define __CAN_PROTOCOL_H

#include "stm32f1xx_hal.h"

// CAN ID定义
#define MOTOR_X_CMD_ID   0x201
#define MOTOR_Y_CMD_ID   0x202
#define MOTOR_Z_CMD_ID   0x203
#define HEARTBEAT_ID     0x200

// 函数声明
void send_motor_enable(uint8_t motor_id, uint8_t enable);
void send_motor_command(uint8_t motor_id, float torque);
void send_heartbeat(void);
void can_receive_handler(uint32_t id, uint8_t* data);

#endif
