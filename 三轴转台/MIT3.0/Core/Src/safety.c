#include "safety.h"
#include "config.h"
#include "dm4310_driver.h"

// 安全检查
uint8_t Safety_Check(void) {
    for (int i = 0; i < MOTOR_COUNT; i++) {
        // 温度保护
        if (Motors[i].temperature > MAX_TEMP) {
            return SAFETY_ERROR_TEMP;
        }
        
        // 电流保护
        if (fabsf(Motors[i].current) > MAX_CURRENT) {
            return SAFETY_ERROR_CURRENT;
        }
        
        // 位置偏差保护
        float pos_error = fabsf(Motors[i].target_position - Motors[i].position);
        if (pos_error > 10.0f) {  // 10度位置偏差
            return SAFETY_ERROR_POSITION;
        }
        
        // 电机错误状态
        if (Motors[i].error != 0) {
            return Motors[i].error;
        }
    }
    return SAFETY_OK;
}

// 紧急停止处理
void Emergency_Stop(void) {
    // 发送零指令给所有电机
    for (int i = 0; i < MOTOR_COUNT; i++) {
        Motors[i].target_position = Motors[i].position;
        Motors[i].target_velocity = 0;
        Motors[i].target_current = 0;
        DM4310_SetTarget(Motors[i].id, MIT_MODE_CURRENT, 0, 0, 0);
    }
    
    SystemState = SYS_EMERGENCY_STOP;
    
    // 闪烁LED指示紧急状态
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_Delay(500);
}
