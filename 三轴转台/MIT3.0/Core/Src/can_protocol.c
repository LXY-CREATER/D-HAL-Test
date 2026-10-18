#include "can_protocol.h"
#include "system_state.h"
#include "motor_control.h"
#include "can.h"
#include <stdio.h>  // 添加printf支持

// MIT 模式专用定义
#define MIT_MOTOR_ENABLE    0xFFFF
#define MIT_MOTOR_DISABLE   0x0000
#define MIT_TORQUE_SCALE    1000.0f  // 扭矩缩放因子 (Nm * 1000)

// 调试宏定义
#define DEBUG_ENABLE 1
#if DEBUG_ENABLE
#define CAN_DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define CAN_DEBUG_PRINTF(...)
#endif

// 发送电机使能/失能命令 (MIT模式)
void send_motor_enable(uint8_t motor_id, uint8_t enable) {
    CAN_TxHeaderTypeDef tx_header = {0};
    uint8_t tx_data[8] = {0};
    uint32_t tx_mailbox;
    
    // MIT模式使能命令格式：16位使能标志 + 16位零
    uint16_t enable_cmd = enable ? MIT_MOTOR_ENABLE : MIT_MOTOR_DISABLE;
    tx_data[0] = enable_cmd >> 8;    // 高字节
    tx_data[1] = enable_cmd & 0xFF;  // 低字节
      // 其余字节清零
    memset(&tx_data[2], 0, 6);
		
    // 设置CAN头
//		uint32_t std_id = 0;
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
            CAN_DEBUG_PRINTF("Invalid motor ID: %d\n", motor_id);
            return;
    }
    
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.IDE = CAN_ID_STD;
    tx_header.DLC = 8;  // MIT模式通常使用8字节
    tx_header.TransmitGlobalTime = DISABLE;
    
    // 调试输出
    printf("MIT: Sending %s to motor %d (ID: 0x%03X)\n", 
                     enable ? "ENABLE" : "DISABLE", 
                     motor_id,
                     tx_header.StdId);
    
    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(&hcan, &tx_header, tx_data, &tx_mailbox);
    if(status != HAL_OK) {
        printf("CAN send error: %d\n", status);
    }
}

// 发送电机扭矩命令 (MIT模式)
void send_motor_command(uint8_t motor_id, float torque) {
    // 如果系统处于紧急停止状态，不发送扭矩命令
    if(g_system.current_state == SYS_STATE_EMERGENCY_STOP) {
        return;
    }
    
    CAN_TxHeaderTypeDef tx_header = {0};
    uint8_t tx_data[8] = {0};
    uint32_t tx_mailbox;
    
    // MIT模式扭矩命令格式
    // 字节0-1: 期望扭矩 (int16, 单位: Nm * 1000)
    // 字节2-3: 期望位置 (int16, MIT模式通常不使用)
    // 字节4-5: Kp (int16, 位置环增益)
    // 字节6-7: Kd (int16, 速度环增益)
    
    // 转换为MIT格式 (扭矩值缩放)
    int16_t torque_int = (int16_t)(torque * MIT_TORQUE_SCALE);
    tx_data[0] = torque_int >> 8;
    tx_data[1] = torque_int & 0xFF;
    
    // 位置值 (MIT模式通常设为0)
    int16_t position_int = 0;
    tx_data[2] = position_int >> 8;
    tx_data[3] = position_int & 0xFF;
    
    // Kp和Kd (根据需求设置)
    int16_t kp = 0;  // 位置环增益 (可配置)
    int16_t kd = 0;  // 速度环增益 (可配置)
    tx_data[4] = kp >> 8;
    tx_data[5] = kp & 0xFF;
    tx_data[6] = kd >> 8;
    tx_data[7] = kd & 0xFF;
    
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
            CAN_DEBUG_PRINTF("Invalid motor ID: %d\n", motor_id);
            return;
    }
    
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.IDE = CAN_ID_STD;
    tx_header.DLC = 8;  // MIT模式使用8字节
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
    
    tx_data[0] = (uint8_t)g_system.current_state; // 发送当前系统状态
    
    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(&hcan, &tx_header, tx_data, &tx_mailbox);
    if(status != HAL_OK) {
        CAN_DEBUG_PRINTF("Heartbeat send error: %d\n", status);
    }
}

// CAN接收处理函数 - MIT模式反馈
void can_receive_handler(uint32_t id, uint8_t* data) {
    // MIT模式电机状态反馈 (ID为0x200 + 电机ID)
    if(id >= 0x204 && id <= 0x206) {
        uint8_t motor_id = id - 0x204;
        
        // 解析MIT反馈数据
        int16_t position = (data[0] << 8) | data[1];
        int16_t velocity = (data[2] << 8) | data[3];
        int16_t torque = (data[4] << 8) | data[5];
        uint16_t status = (data[6] << 8) | data[7];
        
        // 更新电机使能状态 (根据状态字)
        uint8_t enabled = (status & 0x01); // 位0: 使能状态
        g_system.motor_enabled[motor_id] = enabled;
        
        printf("Motor %d: Pos=%d, Vel=%d, Torque=%.2fNm, Status=0x%04X\n", 
               motor_id, position, velocity, torque / MIT_TORQUE_SCALE, status);
    }
    else {
        printf("CAN RX: ID=0x%03lX, Data: %02X %02X %02X %02X %02X %02X %02X %02X\n",
               id, data[0], data[1], data[2], data[3], 
               data[4], data[5], data[6], data[7]);
    }
}
