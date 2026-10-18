#include "sero.h"
#include "tim.h"
#include "usart.h"
void Servo_SetAngle(TIM_HandleTypeDef *htim, uint32_t channel, float angle) {
    if (angle > 180) angle = 180;
    if (angle < 0) angle = 0;
    uint16_t pulse = (uint16_t)(500 + (angle / 180) * 2000); // 0.5ms ~ 2.5ms
    __HAL_TIM_SET_COMPARE(htim, channel, pulse);
}

// ¶æ»ú²âÊÔº¯Êý
void Servo_Test(void) {
    for (int i = 0; i <= 180; i += 30) {
        char msg[40];
        snprintf(msg, sizeof(msg), "Setting servo to %d degrees\r\n", i);
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
        
        Servo_SetAngle(&htim4, TIM_CHANNEL_3, i);
        HAL_Delay(1000);
    }
    Servo_SetAngle(&htim4, TIM_CHANNEL_3, 0);
}
