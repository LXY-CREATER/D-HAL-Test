#include "dm4310_driver.h"
#include "config.h"

// 达妙4310电机控制指令
void DM4310_SetTarget(uint8_t motor_id, uint8_t mode, float position, float velocity, float current) {
    CAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data[8];
    uint32_t tx_mailbox;
    
    // 填充数据
    tx_data[0] = mode;  // 控制模式
    
    // 位置 (0.01度/LSB)
    int16_t pos_val = (int16_t)(position * 100);
    tx_data[1] = (uint8_t)(pos_val & 0xFF);
    tx_data[2] = (uint8_t)((pos_val >> 8) & 0xFF);
    
    // 速度 (0.01dps/LSB)
    int16_t vel_val = (int16_t)(velocity * 100);
    tx_data[3] = (uint8_t)(vel_val & 0xFF);
    tx_data[4] = (uint8_t)((vel_val >> 8) & 0xFF);
    
    // 电流 (0.01A/LSB)
    int16_t cur_val = (int16_t)(current * 100);
    tx_data[5] = (uint8_t)(cur_val & 0xFF);
    tx_data[6] = (uint8_t)((cur_val >> 8) & 0xFF);
    
    tx_data[7] = 0;  // 保留字节
    
    // 配置CAN帧头
    tx_header.StdId = motor_id;
    tx_header.ExtId = 0;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.IDE = CAN_ID_STD;
    tx_header.DLC = 8;
    tx_header.TransmitGlobalTime = DISABLE;
    
    // 发送CAN消息
    if (HAL_CAN_AddTxMessage(&hcan, &tx_header, tx_data, &tx_mailbox) != HAL_OK) {
        Error_Handler();
    }
}

// 处理电机反馈数据
void DM4310_ProcessFeedback(uint32_t std_id, uint8_t *data) {
    uint8_t motor_index = 0;
    
    // 确定电机索引
    switch (std_id) {
        case CAN_MASTER_ID:
            motor_index = 0;
            break;
        case CAN_SLAVE1_ID:
            motor_index = 1;
            break;
        case CAN_SLAVE2_ID:
            motor_index = 2;
            break;
        default:
            return;  // 未知ID
    }
    
    // 解析反馈数据
    Motors[motor_index].position = (float)((int16_t)((data[1] << 8) | data[0]) * 0.01f);
    Motors[motor_index].velocity = (float)((int16_t)((data[3] << 8) | data[2]) * 0.01f);
    Motors[motor_index].current = (float)((int16_t)((data[5] << 8) | data[4]) * 0.01f);
    Motors[motor_index].temperature = (float)data[6];
    Motors[motor_index].error = data[7];
}

// 发送同步控制指令
void DM4310_SendSyncCommands(void) {
    for (int i = 0; i < MOTOR_COUNT; i++) {
        DM4310_SetTarget(
            Motors[i].id,
            Motors[i].mode,
            Motors[i].target_position,
            Motors[i].target_velocity,
            Motors[i].target_current
        );
    }
}

// 电机初始化
void DM4310_Init(void) {
    // 初始化主轴 (横滚轴)
    Motors[0].id = CAN_MASTER_ID;
    Motors[0].mode = MIT_MODE_POSITION;
    Motors[0].position = 0.0f;
    Motors[0].velocity = 0.0f;
    Motors[0].current = 0.0f;
    Motors[0].temperature = 0.0f;
    Motors[0].target_position = 0.0f;
    Motors[0].target_velocity = 0.0f;
    Motors[0].target_current = 0.0f;
    Motors[0].error = 0;
    
    // 初始化从轴1 (俯仰轴)
    Motors[1].id = CAN_SLAVE1_ID;
    Motors[1].mode = MIT_MODE_POSITION;
    Motors[1].position = 0.0f;
    Motors[1].velocity = 0.0f;
    Motors[1].current = 0.0f;
    Motors[1].temperature = 0.0f;
    Motors[1].target_position = 0.0f;
    Motors[1].target_velocity = 0.0f;
    Motors[1].target_current = 0.0f;
    Motors[1].error = 0;
    
    // 初始化从轴2 (偏航轴)
    Motors[2].id = CAN_SLAVE2_ID;
    Motors[2].mode = MIT_MODE_POSITION;
    Motors[2].position = 0.0f;
    Motors[2].velocity = 0.0f;
    Motors[2].current = 0.0f;
    Motors[2].temperature = 0.0f;
    Motors[2].target_position = 0.0f;
    Motors[2].target_velocity = 0.0f;
    Motors[2].target_current = 0.0f;
    Motors[2].error = 0;
}
