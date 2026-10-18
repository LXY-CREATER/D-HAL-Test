//#include "Inputcapture.h"
//#include "tim.h"

//float distance = 0;

//void Inputcapture_Init(void) {
//    HAL_TIM_Base_Start_IT(&htim1);
//    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
//    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);
//}

//float Inputcapture_GetDistance(void) {
//    // 触发测距
//    HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_SET);
//    HAL_Delay(1);
//    HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_RESET);
//    __HAL_TIM_SET_COUNTER(&htim1, 0);
//    HAL_Delay(20);  //等待结果

//    return distance;
//}

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
//    if (htim == &htim1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
//        int upEdge = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
//        int downEdge = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
//        //捕获后需手动清除中断标志，否则可能重复计数（中断清除函数）  
//			  __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_CC1| TIM_FLAG_CC2);
//			  //处理溢出
//        float edige;
//        static uint32_t overflow_count = 0;
//        if (downEdge < upEdge) {
//            edige = (0xFFFF - upEdge) + downEdge + (overflow_count * 0x10000);
//        } else {
//            edige = downEdge - upEdge;
//        }

//        distance = (edige * 0.034) / 2;  //测距完成，单位厘米 
//    }
//}
#include "Inputcapture.h"
#include "tim.h"
#include "usart.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"
//全局变量
volatile float distance = 0.0;
volatile uint8_t measurement_done = 0;
volatile uint32_t t1 = 0, t2 = 0;
volatile uint32_t overflow_count = 0;

void Inputcapture_Init(void) {//初始化时启动上升沿捕获
    HAL_TIM_Base_Start_IT(&htim1);
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_3);
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);
   
}

void TIM1_Delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

float Inputcapture_GetDistance(void) {
    measurement_done = 0;
    // 发送10us的Trige脉冲
    HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_SET);
    TIM1_Delay_us(10);
    HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_RESET);
    
    // 等待测量完成或超时
    uint32_t timeout = HAL_GetTick() + 30; // 30ms超时
   while (!measurement_done && HAL_GetTick() < timeout);
    return distance;
}

// 输入捕获中断回调
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM1) {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
            // 使用宏获取当前极性
            uint32_t polarity = __HAL_TIM_GET_CAPTUREPOLARITY(htim, TIM_CHANNEL_3);
            if (polarity == TIM_INPUTCHANNELPOLARITY_RISING) {
                // 处理上升沿
                t1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_FALLING);
                overflow_count = 0;
            } else {
                // 处理下降沿
                t2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
                uint32_t total_ticks = (overflow_count * 0xFFFF) + (t2 - t1);
                distance = (total_ticks * 0.034) / 2; // 单位：厘米
                measurement_done = 1;
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);
            }
        }
    }
}// 定时器溢出处理
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM1) {
        overflow_count++;
    }
}
