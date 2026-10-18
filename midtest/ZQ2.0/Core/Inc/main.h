/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_1_Pin GPIO_PIN_0
#define LED_1_GPIO_Port GPIOF
#define LED_2_Pin GPIO_PIN_1
#define LED_2_GPIO_Port GPIOF
#define LED_3_Pin GPIO_PIN_2
#define LED_3_GPIO_Port GPIOF
#define LED_4_Pin GPIO_PIN_3
#define LED_4_GPIO_Port GPIOF
#define LED_A_Pin GPIO_PIN_0
#define LED_A_GPIO_Port GPIOC
#define LED_B_Pin GPIO_PIN_1
#define LED_B_GPIO_Port GPIOC
#define LED_C_Pin GPIO_PIN_2
#define LED_C_GPIO_Port GPIOC
#define LED_D_Pin GPIO_PIN_3
#define LED_D_GPIO_Port GPIOC
#define LED_E_Pin GPIO_PIN_4
#define LED_E_GPIO_Port GPIOC
#define LED_F_Pin GPIO_PIN_5
#define LED_F_GPIO_Port GPIOC
#define LED_G_Pin GPIO_PIN_6
#define LED_G_GPIO_Port GPIOC
#define LED_DP_Pin GPIO_PIN_7
#define LED_DP_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */
// 系统模式定义
typedef enum {
    INPUT_MODE,       // 角度输入模式
    POTENTIOMETER_MODE // 电位器控制模式
} SystemMode;

// 按键类型定义（与3*4矩阵对应，10=确认，11=0，12=清除）
typedef enum {
    KEY_NONE = 0,
    KEY_1, KEY_2, KEY_3,
    KEY_4, KEY_5, KEY_6,
    KEY_7, KEY_8, KEY_9,
    KEY_CONFIRM,      // 10: 确认键
    KEY_0,            // 11: 0键
    KEY_CLEAR         // 12: 清除键
} KeyType;

// 全局变量声明（供中断和其他模块调用）
extern SystemMode currentMode;
extern uint16_t inputAngle;  // 当前角度值（显示用）

// 函数原型声明
void System_Init(void);                  // 系统初始化
void Mode_Handle(void);                  // 模式处理主函数
uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max); // 映射函数
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
