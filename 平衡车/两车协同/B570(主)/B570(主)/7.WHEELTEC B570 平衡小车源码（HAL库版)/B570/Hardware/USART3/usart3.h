#ifndef __USRAT3_H
#define __USRAT3_H 
#include "sys.h"

extern u8 Usart2_Receive_buf[1];
extern u8 Usart2_Receive;
extern u8 Usart3_Receive_buf[1];
extern u8 Usart3_Receive;
extern u8 Cooperative_Cmd_Send;
extern s8 Coop_Speed_Base;
extern s8 Coop_Speed_Turn;

#define COOP_MASTER_MARK 0xAA
#define COOP_SLAVE_MARK  0xBB
#define COOP_CMD_FORWARD    0x01
#define COOP_CMD_BACKWARD   0x02
#define COOP_CMD_STOP       0x03
#define COOP_CMD_TURN_LEFT  0x04
#define COOP_CMD_TURN_RIGHT 0x05

void SendCooperativePacket(u8 cmd, s8 speed, float yaw, u8 turn_radius);

#endif