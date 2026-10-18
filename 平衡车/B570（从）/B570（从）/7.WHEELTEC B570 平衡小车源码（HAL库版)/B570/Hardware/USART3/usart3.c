/***********************************************
* ??? USART3 ??? ?? ??????????? v3.0
* USART3 (PB10/PB11) = BT-04  ?? ????????HC-05?????
*
* ??????: [0xAA, cmd, master_speed, yaw_H, yaw_L]
* ?????: [0xBB, slave_speed, yaw_H, yaw_L, flags]
*
* Brand: WHEELTEC  Website: wheeltec.net
* Version: V3.0 Slim (??????????????,???Flash)
***********************************************/

#include "usart3.h"

u8 Usart2_Receive_buf[1];
u8 Usart2_Receive;
u8 Usart3_Receive_buf[1];
u8 Usart3_Receive;

float Master_Target_Speed = 0;
float Master_Target_Yaw   = 0;
u8    Master_Coop_Cmd     = COOP_CMD_STOP;

/**************************************************************************
Function: Parse master cooperative packet (state machine)
Packet: [0xAA, cmd, master_speed(i8), yaw_H, yaw_L]
**************************************************************************/
void ParseMasterPacket(u8 byte)
{
	static u8 state = 0;
	static u8 buf[5];
	static u8 idx = 0;
	static u8 timeout = 0;        // ???????(????+1)

	timeout++;
	if(timeout > 20) {           // 20???????????,????????
		state = 0; idx = 0; timeout = 0;
	}

	switch(state) {
		case 0:
			if(byte == COOP_MASTER_MARK) { state = 1; idx = 0; buf[idx++] = byte; timeout = 0; }
			break;
		case 1:
			buf[idx++] = byte;
			if(idx >= 5) {
				Master_Coop_Cmd    = buf[1];
				Master_Target_Speed = (s8)buf[2];
				Master_Target_Yaw   = ((s16)(buf[3] << 8) | buf[4]) / 10.0f;
				state = 0; idx = 0; timeout = 0;
				LED=!LED;  // ????????????????LED, ???????=???????
			}
			break;
		default: state = 0; idx = 0; timeout = 0; break;
	}
}

/**************************************************************************
Function: Send slave feedback to master
Packet: [0xBB, slave_speed(i8), yaw_H, yaw_L, flags]
Called from main loop, every ~200ms
**************************************************************************/
void SendSlaveFeedback(void)
{
	u8  packet[5];
	s8  speed_byte;
	s16 yaw_scaled;
	float avg_speed;

	packet[0] = COOP_SLAVE_MARK;
	avg_speed = (Velocity_Left + Velocity_Right) / 2.0f;
	if(avg_speed > 127.0f)   speed_byte = 127;
	else if(avg_speed < -127.0f) speed_byte = -127;
	else                     speed_byte = (s8)avg_speed;
	packet[1] = (u8)speed_byte;

	yaw_scaled = (s16)(Yaw * 10.0f);
	packet[2] = (u8)((yaw_scaled >> 8) & 0xFF);
	packet[3] = (u8)(yaw_scaled & 0xFF);
	packet[4] = (Flag_Stop == 1) ? 0x01 : 0x00;

	HAL_UART_Transmit(&huart3, packet, 5, 10);
}

/**************************************************************************
Function: HAL UART RX Callback
- USART3: ??????????????? + ????????????(??PID/POW)
- USART2: ???????(????,????????????????)
**************************************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	if(UartHandle->Instance == USART3)
	{
		Usart3_Receive = Usart3_Receive_buf[0];

		if(Mode == Cooperative_Mode)
		{
			LED=!LED;

			ParseMasterPacket((u8)Usart3_Receive);
			switch(Master_Coop_Cmd) {
				case COOP_CMD_FORWARD:    Flag_front=1;Flag_back=0;Flag_Left=0;Flag_Right=0; break;
				case COOP_CMD_BACKWARD:   Flag_front=0;Flag_back=1;Flag_Left=0;Flag_Right=0; break;
				case COOP_CMD_STOP:       Flag_front=0;Flag_back=0;Flag_Left=0;Flag_Right=0; break;
				case COOP_CMD_TURN_LEFT:  Flag_front=0;Flag_back=0;Flag_Left=1;Flag_Right=0; break;
				case COOP_CMD_TURN_RIGHT: Flag_front=0;Flag_back=0;Flag_Left=0;Flag_Right=1; break;
				default: break;
			}
		}
		else
		{
			u8 r = (u8)Usart3_Receive;
			if(r==0x59) Flag_velocity=2;
			if(r==0x58) Flag_velocity=1;
			if(r>10) {
				if(r==0x5A)       Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=0;
				else if(r==0x41)  Flag_front=1,Flag_back=0,Flag_Left=0,Flag_Right=0;
				else if(r==0x45)  Flag_front=0,Flag_back=1,Flag_Left=0,Flag_Right=0;
				else if(r==0x42||r==0x43||r==0x44) Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=1;
				else if(r==0x46||r==0x47||r==0x48) Flag_front=0,Flag_back=0,Flag_Left=1,Flag_Right=0;
				else              Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=0;
			}
			if(r<10) {
				if(r==0x00)       Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=0;
				else if(r==0x01)  Flag_front=1,Flag_back=0,Flag_Left=0,Flag_Right=0;
				else if(r==0x05)  Flag_front=0,Flag_back=1,Flag_Left=0,Flag_Right=0;
				else if(r>=0x02&&r<=0x04) Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=1;
				else if(r>=0x06&&r<=0x08) Flag_front=0,Flag_back=0,Flag_Left=1,Flag_Right=0;
				else              Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=0;
			}
		}
		HAL_UART_Receive_IT(&huart3, Usart3_Receive_buf, sizeof(Usart3_Receive_buf));
	}
}
