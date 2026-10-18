//#ifndef INPUTCAPTURE_H
//#define INPUTCAPTURE_H

//#include "main.h"


//void Inputcapture_Init(void);
//float Inputcapture_GetDistance(void);

//extern float distance;
//	
//#endif
#ifndef INPUTCAPTURE_H
#define INPUTCAPTURE_H

#include "main.h"

void Inputcapture_Init(void);
float Inputcapture_GetDistance(void);
void TIM1_Delay_us(uint16_t us);

extern volatile float distance;
extern volatile uint8_t measurement_done;

// 在 inputcapture.h 或 inputcapture.c 中添加以下宏
#define __HAL_TIM_GET_CAPTUREPOLARITY(__HANDLE__, __CHANNEL__) \
  (((__CHANNEL__) == TIM_CHANNEL_1) ? ((__HANDLE__)->Instance->CCER & TIM_CCER_CC1P) : \
   ((__CHANNEL__) == TIM_CHANNEL_2) ? ((__HANDLE__)->Instance->CCER & TIM_CCER_CC2P) : \
   ((__CHANNEL__) == TIM_CHANNEL_3) ? ((__HANDLE__)->Instance->CCER & TIM_CCER_CC3P) : \
   ((__HANDLE__)->Instance->CCER & TIM_CCER_CC4P))

#endif
