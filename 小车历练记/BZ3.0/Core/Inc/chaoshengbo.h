#ifndef CHAOSHENGBO_CHAOSHENGBO_H_
#define CHAOSHENGBO_CHAOSHENGBO_H_

#include "stm32f1xx_hal.h" //HAL库文件声明
#include <main.h>
//#include "../delay/delay.h"  //包含 delay.h 头文件，该文件可能定义了用于延时的函数和宏，../ 表示返回上一级目录
#include "delay.h"

#define TRIG_ON  HAL_GPIO_WritePin(GPIOB, TRIG_Pin, GPIO_PIN_SET)   //定义TRIG输出高电平
#define TRIG_OFF  HAL_GPIO_WritePin(GPIOB, TRIG_Pin, GPIO_PIN_RESET)//定义TRIG输出低电平
extern TIM_HandleTypeDef htim4;//声明TIM4的HAL库结构体
extern TIM_HandleTypeDef htim3;//声明TIM3的HAL库结构体

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);//定时器中断回调函数
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);//定时器输入捕获中断回调函数

#endif /* CHAOSHENGBO_CHAOSHENGBO_H_ */
