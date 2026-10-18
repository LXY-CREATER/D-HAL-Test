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
#define L298N_Pin GPIO_PIN_0
#define L298N_GPIO_Port GPIOA
#define SG90_Pin GPIO_PIN_1
#define SG90_GPIO_Port GPIOA
#define IN22_Pin GPIO_PIN_0
#define IN22_GPIO_Port GPIOB
#define IN33_Pin GPIO_PIN_1
#define IN33_GPIO_Port GPIOB
#define IN44_Pin GPIO_PIN_2
#define IN44_GPIO_Port GPIOB
#define TRIG_Pin GPIO_PIN_14
#define TRIG_GPIO_Port GPIOB
#define IN1_Pin GPIO_PIN_8
#define IN1_GPIO_Port GPIOA
#define IN2_Pin GPIO_PIN_12
#define IN2_GPIO_Port GPIOA
#define IN3_Pin GPIO_PIN_13
#define IN3_GPIO_Port GPIOA
#define IN4_Pin GPIO_PIN_14
#define IN4_GPIO_Port GPIOA
#define IN11_Pin GPIO_PIN_15
#define IN11_GPIO_Port GPIOA
#define ECHO_Pin GPIO_PIN_9
#define ECHO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
