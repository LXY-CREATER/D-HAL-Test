/***********************************************
* ????????? by WHEELTEC
* USART2 (PA2/PA3) = JDY-33 ??????? ?? ???????APP  
* USART3 (PB10/PB11) = HC-05 ???????? ?? ??????BT-04
*
* ??????? v3.0 (??????):
*   ??????: [0xAA, cmd, master_speed, yaw_H, yaw_L]
*   ?????: [0xBB, slave_speed, yaw_H, yaw_L, flags]
*
* Brand: WHEELTEC
* Website: wheeltec.net
* Version: V3.0 (Cooperative Transport)
* Update: 2026-07
***********************************************/

#include "usart3.h"

// ========== ???????? ==========
u8 Usart2_Receive_buf[1];          //USART2(JDY-33)?????????????
u8 Usart2_Receive;                 //??JDY-33?????????
u8 Usart3_Receive_buf[1];          //USART3(HC-05)?????????????
u8 Usart3_Receive;                 //??HC-05?????????
u8 Cooperative_Cmd_Send=0;         //????????????????(0=??)
u8 Last_Sent_Cmd;                  //?????????????

// ========== ???????? (???????) ==========
float Slave_Actual_Speed = 0;      // ????????? (mm/s)
float Slave_Actual_Yaw   = 0;      // ????????? (deg)
u8    Slave_Flags = 0;             // ????????
u8    Slave_Online = 0;            // ?????????
u8    Slave_Timeout = 0;           // ???????(?50ms+1,???????,>40=????)

/**************************************************************************
Function: Send cooperative packet to slave car (new protocol)
Packet: [0xAA, cmd, master_speed(i8), yaw_H, yaw_L]
  - cmd: 0x01=Forward, 0x02=Backward, 0x03=Stop, 0x04=TurnLeft, 0x05=TurnRight
  - master_speed: ??????????(mm/s), int8_t, -127~127
  - yaw_H,yaw_L: ????????????*10, int16_t, big-endian
**************************************************************************/
void SendCooperativePacket(u8 cmd)
{
	u8  packet[5];
	s8  speed_byte;
	s16 yaw_scaled;
	float avg_speed;

	packet[0] = COOP_MASTER_MARK;           // ?????? 0xAA
	packet[1] = cmd;                         // ????

	// ??????????(???????? mm/s) ?? s8
	avg_speed = (Velocity_Left + Velocity_Right) / 2.0f;
	if(avg_speed > 127.0f)   speed_byte = 127;
	else if(avg_speed < -127.0f) speed_byte = -127;
	else                     speed_byte = (s8)avg_speed;
	packet[2] = (u8)speed_byte;

	// ???????????(Yaw deg*10) ?? s16 big-endian
	yaw_scaled = (s16)(Yaw * 10.0f);
	packet[3] = (u8)((yaw_scaled >> 8) & 0xFF);   // ?????
	packet[4] = (u8)(yaw_scaled & 0xFF);           // ?????

	HAL_UART_Transmit(&huart3, packet, 5, 10);
}

/**************************************************************************
Function: Send single-byte command (old protocol, kept for compatibility)
**************************************************************************/
void SendCooperativeCmd(u8 cmd)
{
	HAL_UART_Transmit(&huart3, &cmd, 1, 10);
}

/**************************************************************************
Function: Parse slave feedback packet (state machine)
Packet: [0xBB, slave_speed(i8), yaw_H, yaw_L, flags]
Called byte-by-byte from USART3 RX callback in cooperative mode
**************************************************************************/
void ParseSlavePacket(u8 byte)
{
	static u8 parse_state = 0;
	static u8 pkt_buf[5];
	static u8 pkt_idx = 0;

	switch(parse_state)
	{
		case 0:  // ?????????
			if(byte == COOP_SLAVE_MARK) {
				parse_state = 1;
				pkt_idx = 0;
				pkt_buf[pkt_idx++] = byte;
			}
			break;
		case 1:  // ???????1~4
			pkt_buf[pkt_idx++] = byte;
			if(pkt_idx >= 5) {
				// ??????????
				Slave_Actual_Speed = (s8)pkt_buf[1];           // mm/s
				Slave_Actual_Yaw   = ((s16)(pkt_buf[2] << 8) | pkt_buf[3]) / 10.0f;  // deg
				Slave_Flags        = pkt_buf[4];
				Slave_Online       = 1;
				Slave_Timeout      = 0;
				parse_state = 0;
			}
			break;
		default:
			parse_state = 0;
			break;
	}
}

/**************************************************************************
Function: HAL UART RX Callback (shared for USART2 & USART3)
- USART2(JDY-33): ???APP??????(????/??/PID)
- USART3(HC-05):  ????????????????
**************************************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	//============ USART2: JDY-33 ???APP??? ============//
	if(UartHandle->Instance == USART2)
	{
		static u8 Flag_PID,i,j,Receive[50];
		static float Data;
		u8 uart_receive = Usart2_Receive_buf[0];
		Usart2_Receive = uart_receive;
		
		//--- ?????? ---//
		if(uart_receive==0x59)  Flag_velocity=2;
		if(uart_receive==0x58)  Flag_velocity=1;
		
		//--- ???????(??????? A~H), ?????????, ???????????????? ---//
		if(uart_receive>10)
		{
			if(uart_receive==0x5A)       Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=0,Slave_Turn_Left=0,Slave_Turn_Right=0;
			else if(uart_receive==0x41)  Flag_front=1,Flag_back=0,Flag_Left=0,Flag_Right=0;
			else if(uart_receive==0x45)  Flag_front=0,Flag_back=1,Flag_Left=0,Flag_Right=0;
			else if(uart_receive==0x42||uart_receive==0x43)  // B/C???????????
			                             Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=1;
			else if(uart_receive==0x47||uart_receive==0x48)  // G/H???????????
			                             Flag_front=0,Flag_back=0,Flag_Left=1,Flag_Right=0;
			else if(uart_receive==0x44)  // D??????????
			                             Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=0,Slave_Turn_Left=0,Slave_Turn_Right=1;
			else if(uart_receive==0x46)  // F??????????
			                             Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=0,Slave_Turn_Left=1,Slave_Turn_Right=0;
			// ????????: ????????, ?????
		}
		
		//--- ???????(??????? 0~8, MiniBalance V1.0) ---//
		if(uart_receive<10)
		{
			if(uart_receive==0x00)       Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=0;
			else if(uart_receive==0x01)  Flag_front=1,Flag_back=0,Flag_Left=0,Flag_Right=0;
			else if(uart_receive==0x05)  Flag_front=0,Flag_back=1,Flag_Left=0,Flag_Right=0;
			else if(uart_receive==0x02||uart_receive==0x03||uart_receive==0x04)
			                             Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=1;
			else if(uart_receive==0x06||uart_receive==0x07||uart_receive==0x08)
			                             Flag_front=0,Flag_back=0,Flag_Left=1,Flag_Right=0;
			// ????????: ????????, ?????
		}
		
		//--- ?????? ---//
		if(uart_receive==0x4D)  Mode = Cooperative_Mode;   //'M'?????????
		if(uart_receive==0x4E)  Mode = Normal_Mode;        //'N'???????
		
		//--- PID???????????? { } ---//
		if(uart_receive==0x7B)  Flag_PID=1;
		if(uart_receive==0x7D)  Flag_PID=2;
		
		if(Flag_PID==1)
		{
			Receive[i] = uart_receive;
			i++;
		}
		if(Flag_PID==2)
		{
			if(Receive[3]==0x50)        PID_Send=1;
			else if(Receive[3]==0x57)   Flash_Send=1;
			else if(Receive[1]!=0x23)
			{
				for(j=i;j>=4;j--)
					Data += (Receive[j-1]-48)*pow(10,i-j);
				switch(Receive[1])
				{
					case 0x30:  Balance_Kp=Data;   break;
					case 0x31:  Balance_Kd=Data;   break;
					case 0x32:  Velocity_Kp=Data;  break;
					case 0x33:  Velocity_Ki=Data;  break;
					case 0x34:  Turn_Kp=Data;      break;
					case 0x35:  Turn_Kd=Data;      break;
					case 0x36:  break;
					case 0x37:  break;
					case 0x38:  break;
				}
			}
			Flag_PID = 0;
			i = 0; j = 0; Data = 0;
			memset(Receive, 0, sizeof(u8)*50);
		}
		
		HAL_UART_Receive_IT(&huart2, Usart2_Receive_buf, sizeof(Usart2_Receive_buf));
	}
	
	//============ USART3: HC-05 ??????(??????????) ============//
	else if(UartHandle->Instance == USART3)
	{
		Usart3_Receive = Usart3_Receive_buf[0];
		if(Mode == Cooperative_Mode) {
			ParseSlavePacket(Usart3_Receive_buf[0]);  // ??????????????
		}
		HAL_UART_Receive_IT(&huart3, Usart3_Receive_buf, sizeof(Usart3_Receive_buf));
	}											 
}
