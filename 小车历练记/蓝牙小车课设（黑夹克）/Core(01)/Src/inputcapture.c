#include "inputcapture.h"
#include "tim.h"
#include "usart.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"
#include <string.h>

// 全局变量
volatile float distance = 0.0;
volatile uint8_t measurement_done = 0;
volatile uint32_t t1 = 0, t2 = 0;
volatile uint32_t overflow_count = 0;
volatile uint8_t measurement_timeout = 0;

// 距离滤波相关变量
#define FILTER_SIZE 5
static float distance_history[FILTER_SIZE] = {0};
static uint8_t history_index = 0;

volatile UltrasoundState us_state = US_STATE_IDLE;

void Inputcapture_Init(void) {
    // 初始化时启动上升沿捕获
    HAL_TIM_Base_Start_IT(&htim1);
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
    
    // 初始化距离历史数组
    memset(distance_history, 0, sizeof(distance_history));
    history_index = 0;
    
    us_state = US_STATE_IDLE;
}

void TIM1_Delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

// 中值滤波函数
float MedianFilter(float new_value) {
    // 更新历史数据
    distance_history[history_index] = new_value;
    history_index = (history_index + 1) % FILTER_SIZE;
    
    // 复制数组进行排序
    float temp[FILTER_SIZE];
    memcpy(temp, distance_history, sizeof(distance_history));
    
    // 简单冒泡排序
    for (int i = 0; i < FILTER_SIZE - 1; i++) {
        for (int j = 0; j < FILTER_SIZE - i - 1; j++) {
            if (temp[j] > temp[j + 1]) {
                float swap = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = swap;
            }
        }
    }
    
    // 返回中值
    return temp[FILTER_SIZE / 2];
}

float Inputcapture_GetDistance(void) {
    measurement_done = 0;
    measurement_timeout = 0;
    us_state = US_STATE_WAITING_RISING;
    
    // 发送10us的Trigger脉冲
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
    TIM1_Delay_us(10);
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
    
    // 等待测量完成或超时
    uint32_t timeout = HAL_GetTick() + 50; // 50ms超时
    while (!measurement_done && !measurement_timeout && HAL_GetTick() < timeout) {
        // 空循环等待
    }
    
    if (measurement_timeout) {
        // 测量超时，返回无效距离
        return -1.0;
    }
    
    // 应用滤波并返回距离
    return MedianFilter(distance);
}

// 输入捕获中断回调
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM1) {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
            // 使用宏获取当前极性
            uint32_t polarity = __HAL_TIM_GET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1);
            
            if (polarity == TIM_INPUTCHANNELPOLARITY_RISING) {
                // 处理上升沿
                t1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
                overflow_count = 0;
                us_state = US_STATE_WAITING_FALLING;
            } else {
                // 处理下降沿
                t2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
                uint32_t total_ticks = (overflow_count * 0xFFFF) + (t2 - t1);
                
                // 计算距离（单位：厘米）
                // 声速约为340m/s = 0.034cm/μs
                // 往返时间，所以除以2
                float raw_distance = (total_ticks * 0.034) / 2;
                
                // 有效性检查（距离应在2cm-400cm之间）
                if (raw_distance >= 2.0 && raw_distance <= 400.0) {
                    distance = raw_distance;
                    us_state = US_STATE_MEASUREMENT_DONE;
                } else {
                    us_state = US_STATE_ERROR;
                }
                
                measurement_done = 1;
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
            }
        }
    }
}

// 定时器溢出处理
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM1) {
        overflow_count++;
        
        // 如果溢出次数过多，认为测量超时
        if (overflow_count > 10) { // 约10*65ms = 650ms
            measurement_timeout = 1;
            us_state = US_STATE_ERROR;
            __HAL_TIM_SET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
        }
    }
}

// 获取超声波状态
UltrasoundState Inputcapture_GetState(void) {
    return us_state;
}

// 重置超声波状态
void Inputcapture_Reset(void) {
    us_state = US_STATE_IDLE;
    measurement_done = 0;
    measurement_timeout = 0;
    overflow_count = 0;
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
}
