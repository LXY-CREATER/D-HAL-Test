#include "usart3.h"

u8 Usart2_Receive_buf[1];
u8 Usart3_Receive_buf[1];

#define COOP_MASTER_MARK 0xAA
#define COOP_SLAVE_MARK  0xBB
#define COOP_CMD_FORWARD    0x01
#define COOP_CMD_BACKWARD   0x02
#define COOP_CMD_STOP       0x03
#define COOP_CMD_TURN_LEFT  0x04
#define COOP_CMD_TURN_RIGHT 0x05
#define CAR_DISTANCE_CM     30.0f

u8 Master_Coop_Cmd = COOP_CMD_STOP;
s8 Master_Target_Speed = 0;
float Master_Target_Yaw = 0;
u8 Master_Turn_Radius = 0;

void SendSlaveFeedback(void);

void ParseMasterPacket(u8 byte)
{
    static u8 state = 0;
    static u8 buf[6];
    static u8 idx = 0;
    static u16 timeout = 0;

    timeout++;
    if(timeout > 20) { state = 0; idx = 0; timeout = 0; }

    switch(state) {
        case 0:
            if(byte == COOP_MASTER_MARK) {
                state = 1;
                idx = 0;
                buf[idx++] = byte;
                timeout = 0;
            }
            break;
        case 1:
            buf[idx++] = byte;
            if(idx >= 6) {
                Master_Coop_Cmd = buf[1];
                Master_Target_Speed = (s8)buf[2];
                Master_Target_Yaw = ((s16)(buf[3] << 8) | buf[4]) / 10.0f;
                Master_Turn_Radius = buf[5];

                Flag_front = 0;
                Flag_back = 0;
                Flag_Left = 0;
                Flag_Right = 0;
                switch(Master_Coop_Cmd) {
                    case COOP_CMD_FORWARD:    Flag_front = 1; break;
                    case COOP_CMD_BACKWARD:   Flag_back = 1; break;
                    case COOP_CMD_TURN_LEFT:  Flag_Left = 1; Flag_front = 1; break;
                    case COOP_CMD_TURN_RIGHT: Flag_Right = 1; Flag_front = 1; break;
                    case COOP_CMD_STOP:       break;
                }

                state = 0;
                idx = 0;
                timeout = 0;
            }
            break;
        default:
            state = 0;
            idx = 0;
            timeout = 0;
            break;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    if(UartHandle->Instance == USART3)
    {
        ParseMasterPacket(Usart3_Receive_buf[0]);
        HAL_UART_Receive_IT(&huart3, Usart3_Receive_buf, sizeof(Usart3_Receive_buf));
    }
    else if(UartHandle->Instance == USART2)
    {
        static u8 state = 0;
        static u8 crc_sum = 0;
        static u8 cnt = 0;
        u8 temp_data;
        temp_data = Usart2_Receive_buf[0];
        switch(state)
        {
            case 0:
                if(temp_data == HEADER_0)
                {
                    Pack_Data.header_0 = temp_data;
                    state++;
                    crc_sum += temp_data;
                } else state = 0, crc_sum = 0;
                break;
            case 1:
                if(temp_data == HEADER_1)
                {
                    Pack_Data.header_1 = temp_data;
                    state++;
                    crc_sum += temp_data;
                } else state = 0, crc_sum = 0;
                break;
            case 2:
                if(temp_data == Length_)
                {
                    Pack_Data.ver_len = temp_data;
                    state++;
                    crc_sum += temp_data;
                } else state = 0, crc_sum = 0;
                break;
            case 3: Pack_Data.speed_h = temp_data; state++; crc_sum += temp_data; break;
            case 4: Pack_Data.speed_l = temp_data; state++; crc_sum += temp_data; break;
            case 5: Pack_Data.start_angle_h = temp_data; state++; crc_sum += temp_data; break;
            case 6: Pack_Data.start_angle_l = temp_data; state++; crc_sum += temp_data; break;
            case 7:case 10:case 13:case 16:case 19:case 22:case 25:case 28:
            case 31:case 34:case 37:case 40:case 43:case 46:case 49:case 52:
                Pack_Data.point[cnt].distance_h = temp_data; state++; crc_sum += temp_data; break;
            case 8:case 11:case 14:case 17:case 20:case 23:case 26:case 29:
            case 32:case 35:case 38:case 41:case 44:case 47:case 50:case 53:
                Pack_Data.point[cnt].distance_l = temp_data; state++; crc_sum += temp_data; break;
            case 9:case 12:case 15:case 18:case 21:case 24:case 27:case 30:
            case 33:case 36:case 39:case 42:case 45:case 48:case 51:case 54:
                Pack_Data.point[cnt].Strong = temp_data; state++; crc_sum += temp_data; cnt++; break;
            case 55: Pack_Data.end_angle_h = temp_data; state++; crc_sum += temp_data; break;
            case 56: Pack_Data.end_angle_l = temp_data; state++; crc_sum += temp_data; break;
            case 57:
                Pack_Data.crc = temp_data;
                state = 0; cnt = 0;
                if(crc_sum == Pack_Data.crc) data_process();
                else memset(&Pack_Data,0,sizeof(Pack_Data));
                crc_sum = 0;
                break;
            default: break;
        }
        HAL_UART_Receive_IT(&huart2, Usart2_Receive_buf, sizeof(Usart2_Receive_buf));
    }
}

void SendSlaveFeedback(void)
{
    u8 buf[5];
    buf[0] = COOP_SLAVE_MARK;
    buf[1] = (u8)(Velocity_Left / 10);
    s16 yaw_int = (s16)(Yaw * 10);
    buf[2] = (u8)(yaw_int >> 8);
    buf[3] = (u8)(yaw_int & 0xFF);
    buf[4] = 0x00;
    HAL_UART_Transmit(&huart3, buf, 5, 10);
}
