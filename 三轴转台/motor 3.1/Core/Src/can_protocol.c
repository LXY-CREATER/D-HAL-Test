#include "can_protocol.h"
#include "system_state.h"
#include "motor_control.h"
#include "can.h"

// MIT模式专用定义
#define MIT_TORQUE_SCALE 1000.0f  // 扭矩缩放因子

// 发送电机使能/失能命令
void send_motor_enable(uint8_t motor_id, uint8_t enable) {
    CAN_TxHeaderTypeDef tx_header = {0};
    uint8_t tx_data[8] = {0};
    uint32_t tx_mailbox;
    
    // 组织数据
    tx_data[0] = enable ? MOTOR_ENABLE_CMD : MOTOR_DISABLE_CMD;
    
    // 设置CAN头
    switch(motor_id) {
        case MOTOR_X:
            tx_header.StdId = MOTOR_X_CMD_ID;
            break;
        case MOTOR_Y:
            tx_header.StdId = MOTOR_Y_CMD_ID;
            break;
        case MOTOR_Z:
            tx_header.StdId = MOTOR_Z_CMD_ID;
            break;
        default:
            return;
    }
    
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.IDE = CAN_ID_STD;
    tx_header.DLC = 1;
    tx_header.TransmitGlobalTime = DISABLE;
    
    HAL_CAN_AddTxMessage(&hcan, &tx_header, tx_data, &tx_mailbox);
}

// 发送电机扭矩命令
void send_motor_command(uint8_t motor_id, float torque) {
    // 紧急停止时不发送命令
    if(g_system.current_state == SYS_STATE_EMERGENCY_STOP) {
        return;
    }
    
    CAN_TxHeaderTypeDef tx_header = {0};
    uint8_t tx_data[8] = {0};
    uint32_t tx_mailbox;
    
    // 组织数据 (MIT模式)
    tx_data[0] = MOTOR_TORQUE_CMD;
    int16_t torque_int = (int16_t)(torque * MIT_TORQUE_SCALE);
    tx_data[1] = torque_int >> 8;
    tx_data[2] = torque_int & 0xFF;
    
    // 设置CAN头
    switch(motor_id) {
        case MOTOR_X:
            tx_header.StdId = MOTOR_X_CMD_ID;
            break;
        case MOTOR_Y:
            tx_header.StdId = MOTOR_Y_CMD_ID;
            break;
        case MOTOR_Z:
            tx_header.StdId = MOTOR_Z_CMD_ID;
            break;
        default:
            return;
    }
    
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.IDE = CAN_ID_STD;
    tx_header.DLC = 3;
    tx_header.TransmitGlobalTime = DISABLE;
    
    HAL_CAN_AddTxMessage(&hcan, &tx_header, tx_data, &tx_mailbox);
}

// 发送心跳包
void send_heartbeat(void) {
    CAN_TxHeaderTypeDef tx_header = {0};
    uint8_t tx_data[8] = {0};
    uint32_t tx_mailbox;
    
    tx_header.StdId = HEARTBEAT_ID;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.IDE = CAN_ID_STD;
    tx_header.DLC = 1;
    tx_header.TransmitGlobalTime = DISABLE;
    
    tx_data[0] = (uint8_t)g_system.current_state;
    
    HAL_CAN_AddTxMessage(&hcan, &tx_header, tx_data, &tx_mailbox);
}
