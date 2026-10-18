//	#include "tim.h"
//	#include "main.h"
//	#include "sero.h"
//		//舵机代码
//int i=600;
//void Sero_Move (void)
//{

//__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_3,500);
//		HAL_Delay(1000);
//__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_3,1000);
//		HAL_Delay(1000);
//__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_3,1500);
//		HAL_Delay(1000);
//__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_3,2000);
//		HAL_Delay(1000);
//__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_3,1500);
//		HAL_Delay(1000);
//__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_3,1000);
//		HAL_Delay(1000);
//__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_3,500);
//		HAL_Delay(1000);
#include "sero.h"
#include "tim.h"
#include "usart.h"
void Servo_SetAngle(TIM_HandleTypeDef *htim, uint32_t channel, float angle) {
    if (angle > 180) angle = 180;
    if (angle < 0) angle = 0;
    uint16_t pulse = (uint16_t)(500 + (angle / 180) * 2000); // 0.5ms ~ 2.5ms
    __HAL_TIM_SET_COMPARE(htim, channel, pulse);
}

// 舵机测试函数
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
