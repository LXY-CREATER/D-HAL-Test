#include "usart3.h"

u8 Usart2_Receive_buf[1];
u8 Usart2_Receive;
u8 Usart3_Receive_buf[1];
u8 Usart3_Receive;
u8 Cooperative_Cmd_Send = 0;
s8 Coop_Speed_Base = 20;
s8 Coop_Speed_Turn = 10;

#define COOP_MASTER_MARK 0xAA
#define COOP_SLAVE_MARK  0xBB
#define COOP_CMD_FORWARD    0x01
#define COOP_CMD_BACKWARD   0x02
#define COOP_CMD_STOP       0x03
#define COOP_CMD_TURN_LEFT  0x04
#define COOP_CMD_TURN_RIGHT 0x05

void SendCooperativePacket(u8 cmd, s8 speed, float yaw, u8 turn_radius)
{
    u8 buf[6];
    buf[0] = COOP_MASTER_MARK;
    buf[1] = cmd;
    buf[2] = (u8)speed;
    s16 yaw_int = (s16)(yaw * 10);
    buf[3] = (u8)(yaw_int >> 8);
    buf[4] = (u8)(yaw_int & 0xFF);
    buf[5] = turn_radius;
    HAL_UART_Transmit(&huart3, buf, 6, 10);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    if(UartHandle->Instance == USART2)
    {
        static u8 Flag_PID,i,j,Receive[50];
        static float Data;
        u8 uart_receive = Usart2_Receive_buf[0];
        Usart2_Receive = uart_receive;

        if(uart_receive==0x59)  Flag_velocity=2;
        if(uart_receive==0x58)  Flag_velocity=1;

        if(uart_receive>10)
        {
            if(uart_receive==0x5A)       Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=0;
            else if(uart_receive==0x41)  Flag_front=1,Flag_back=0,Flag_Left=0,Flag_Right=0;
            else if(uart_receive==0x45)  Flag_front=0,Flag_back=1,Flag_Left=0,Flag_Right=0;
            else if(uart_receive==0x42||uart_receive==0x43)
                                         Flag_front=1,Flag_back=0,Flag_Left=0,Flag_Right=1;
            else if(uart_receive==0x47||uart_receive==0x48)
                                         Flag_front=1,Flag_back=0,Flag_Left=1,Flag_Right=0;
            else if(uart_receive==0x2B)   { Coop_Speed_Base += 5; if(Coop_Speed_Base > 50) Coop_Speed_Base = 50; Coop_Speed_Turn = Coop_Speed_Base / 2; }
            else if(uart_receive==0x2D)   { Coop_Speed_Base -= 5; if(Coop_Speed_Base < 12) Coop_Speed_Base = 12; Coop_Speed_Turn = Coop_Speed_Base / 2; }
            else                         Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=0;
        }

        if(uart_receive<10)
        {
            if(uart_receive==0x00)       Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=0;
            else if(uart_receive==0x01)  Flag_front=1,Flag_back=0,Flag_Left=0,Flag_Right=0;
            else if(uart_receive==0x05)  Flag_front=0,Flag_back=1,Flag_Left=0,Flag_Right=0;
            else if(uart_receive==0x02||uart_receive==0x03||uart_receive==0x04)
                                         Flag_front=1,Flag_back=0,Flag_Left=0,Flag_Right=1;
            else if(uart_receive==0x06||uart_receive==0x07||uart_receive==0x08)
                                         Flag_front=1,Flag_back=0,Flag_Left=1,Flag_Right=0;
            else                         Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=0;
        }

        if(uart_receive==0x4D)  Mode = Cooperative_Mode;
        if(uart_receive==0x4E)  Mode = Normal_Mode;

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

        if(Mode == Cooperative_Mode)
        {
            u8 cmd = COOP_CMD_STOP;
            if(Flag_front==1)         cmd = COOP_CMD_FORWARD;
            else if(Flag_back==1)     cmd = COOP_CMD_BACKWARD;
            else if(Flag_Left==1)     cmd = COOP_CMD_TURN_LEFT;
            else if(Flag_Right==1)    cmd = COOP_CMD_TURN_RIGHT;
            else                      cmd = COOP_CMD_STOP;
            if(cmd != Last_Sent_Cmd)
            {
                Cooperative_Cmd_Send = cmd;
                Last_Sent_Cmd = cmd;
            }
        }

        HAL_UART_Receive_IT(&huart2, Usart2_Receive_buf, sizeof(Usart2_Receive_buf));
    }

    else if(UartHandle->Instance == USART3)
    {
        Usart3_Receive = Usart3_Receive_buf[0];
        HAL_UART_Receive_IT(&huart3, Usart3_Receive_buf, sizeof(Usart3_Receive_buf));
    }
}
