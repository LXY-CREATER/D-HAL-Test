#ifndef __CAN_PROTOCOL_H
#define __CAN_PROTOCOL_H

#include "stm32f1xx_hal.h"

// CAN ID定义 (MIT模式)
#define MOTOR_X_CMD_ID 0x201
#define MOTOR_Y_CMD_ID 0x202
#define MOTOR_Z_CMD_ID 0x203

// 电机控制命令定义
#define MOTOR_ENABLE_CMD   0x01
#define MOTOR_DISABLE_CMD  0x02
#define MOTOR_TORQUE_CMD   0x03   //扭矩

//添加心跳包
#define HEARTBEAT_ID 0X200

// 函数声明
void send_motor_command(uint8_t motor_id, float torque);
void send_motor_enable(uint8_t motor_id, uint8_t enable);
void can_receive_handler(uint32_t id, uint8_t* data);
void send_heartbeat(void);

#endif
