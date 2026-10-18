/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#define SPI1_BL_Pin GPIO_PIN_6
#define SPI1_BL_GPIO_Port GPIOA
#define CS1_Pin GPIO_PIN_0
#define CS1_GPIO_Port GPIOB
#define DC1_Pin GPIO_PIN_1
#define DC1_GPIO_Port GPIOB
#define RESET1_Pin GPIO_PIN_2
#define RESET1_GPIO_Port GPIOB
#define SPI2_BL_Pin GPIO_PIN_14
#define SPI2_BL_GPIO_Port GPIOB
#define CS2_Pin GPIO_PIN_8
#define CS2_GPIO_Port GPIOA
#define DC2_Pin GPIO_PIN_9
#define DC2_GPIO_Port GPIOA
#define RESET2_Pin GPIO_PIN_10
#define RESET2_GPIO_Port GPIOA
#define LED1_R_Pin GPIO_PIN_15
#define LED1_R_GPIO_Port GPIOA
#define LED1_Y_Pin GPIO_PIN_3
#define LED1_Y_GPIO_Port GPIOB
#define LED1_G_Pin GPIO_PIN_4
#define LED1_G_GPIO_Port GPIOB
#define LED2_R_Pin GPIO_PIN_5
#define LED2_R_GPIO_Port GPIOB
#define LED2_Y_Pin GPIO_PIN_6
#define LED2_Y_GPIO_Port GPIOB
#define LED2_G_Pin GPIO_PIN_7
#define LED2_G_GPIO_Port GPIOB
#define KEY1_Pin GPIO_PIN_8
#define KEY1_GPIO_Port GPIOB
#define KEY2_Pin GPIO_PIN_9
#define KEY2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
