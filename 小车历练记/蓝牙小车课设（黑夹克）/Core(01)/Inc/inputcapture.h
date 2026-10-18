#ifndef __INPUTCAPTURE_H
#define __INPUTCAPTURE_H

#include "main.h"

// HAL库中没有定义 __HAL_TIM_GET_CAPTUREPOLARITY，手动添加
#ifndef __HAL_TIM_GET_CAPTUREPOLARITY
#define __HAL_TIM_GET_CAPTUREPOLARITY(__HANDLE__, __CHANNEL__) \
  (((__CHANNEL__) == TIM_CHANNEL_1) ? (((__HANDLE__)->Instance->CCER & TIM_CCER_CC1P) ? TIM_INPUTCHANNELPOLARITY_FALLING : TIM_INPUTCHANNELPOLARITY_RISING) : \
   ((__CHANNEL__) == TIM_CHANNEL_2) ? (((__HANDLE__)->Instance->CCER & TIM_CCER_CC2P) ? TIM_INPUTCHANNELPOLARITY_FALLING : TIM_INPUTCHANNELPOLARITY_RISING) : \
   ((__CHANNEL__) == TIM_CHANNEL_3) ? (((__HANDLE__)->Instance->CCER & TIM_CCER_CC3P) ? TIM_INPUTCHANNELPOLARITY_FALLING : TIM_INPUTCHANNELPOLARITY_RISING) : \
   (((__HANDLE__)->Instance->CCER & TIM_CCER_CC4P) ? TIM_INPUTCHANNELPOLARITY_FALLING : TIM_INPUTCHANNELPOLARITY_RISING))
#endif

// 超声波状态枚举
typedef enum {
    US_STATE_IDLE,
    US_STATE_WAITING_RISING,
    US_STATE_WAITING_FALLING,
    US_STATE_MEASUREMENT_DONE,
    US_STATE_ERROR
} UltrasoundState;

// 函数声明
void Inputcapture_Init(void);
float Inputcapture_GetDistance(void);
UltrasoundState Inputcapture_GetState(void);
void Inputcapture_Reset(void);

// 外部变量声明
extern volatile float distance;

#endif
