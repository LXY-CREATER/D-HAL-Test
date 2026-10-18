/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, LED_A_Pin|LED_B_Pin|LED_C_Pin|LED_D_Pin
                          |LED_E_Pin|LED_F_Pin|LED_G_Pin|LED_DP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_1_Pin|LED_2_Pin|LED_3_Pin|LED_4_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : PFPin PFPin PFPin PFPin
                           PFPin PFPin PFPin PFPin */
  GPIO_InitStruct.Pin = LED_A_Pin|LED_B_Pin|LED_C_Pin|LED_D_Pin
                          |LED_E_Pin|LED_F_Pin|LED_G_Pin|LED_DP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PAPin PAPin PAPin PAPin */
  GPIO_InitStruct.Pin = LED_1_Pin|LED_2_Pin|LED_3_Pin|LED_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */
/*通过点亮不同的数码管LED等显示不同的数字的功能函数*/
void Show_Digital_LED(uint8_t num, uint8_t pos)
{
	if(0 == num){
		HAL_GPIO_WritePin(GPIOF, LED_A_Pin|LED_B_Pin|LED_C_Pin|LED_D_Pin|LED_E_Pin|LED_F_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, LED_G_Pin|LED_DP_Pin, GPIO_PIN_SET);
	}else if(1 == num){
		HAL_GPIO_WritePin(GPIOF, LED_B_Pin|LED_C_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, LED_A_Pin|LED_D_Pin|LED_E_Pin|LED_F_Pin|LED_G_Pin|LED_DP_Pin, GPIO_PIN_SET);
	}else if(2 == num){
		HAL_GPIO_WritePin(GPIOF, LED_A_Pin|LED_B_Pin|LED_G_Pin|LED_D_Pin|LED_E_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, LED_C_Pin|LED_F_Pin|LED_DP_Pin, GPIO_PIN_SET);
	}else if(3 == num){
		HAL_GPIO_WritePin(GPIOF, LED_A_Pin|LED_B_Pin|LED_C_Pin|LED_D_Pin|LED_G_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, LED_E_Pin|LED_F_Pin|LED_DP_Pin, GPIO_PIN_SET);
	}else if(4 == num){
		HAL_GPIO_WritePin(GPIOF, LED_F_Pin|LED_G_Pin|LED_B_Pin|LED_C_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, LED_A_Pin|LED_E_Pin|LED_D_Pin|LED_DP_Pin, GPIO_PIN_SET);
	}else if(5 == num){
		HAL_GPIO_WritePin(GPIOF, LED_A_Pin|LED_C_Pin|LED_D_Pin|LED_F_Pin|LED_G_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, LED_B_Pin|LED_E_Pin|LED_DP_Pin, GPIO_PIN_SET);
	}else if(6 == num){
		HAL_GPIO_WritePin(GPIOF, LED_A_Pin|LED_C_Pin|LED_D_Pin|LED_E_Pin|LED_F_Pin|LED_G_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, LED_B_Pin|LED_DP_Pin, GPIO_PIN_SET);
	}else if(7 == num){
		HAL_GPIO_WritePin(GPIOF, LED_A_Pin|LED_B_Pin|LED_C_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, LED_D_Pin|LED_E_Pin|LED_F_Pin|LED_G_Pin|LED_DP_Pin, GPIO_PIN_SET);
	}else if(8 == num){
		HAL_GPIO_WritePin(GPIOF, LED_A_Pin|LED_B_Pin|LED_C_Pin|LED_D_Pin|LED_E_Pin|LED_F_Pin|LED_G_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, LED_DP_Pin, GPIO_PIN_SET);
	}else if(9 == num){
		HAL_GPIO_WritePin(GPIOF, LED_A_Pin|LED_B_Pin|LED_C_Pin|LED_D_Pin|LED_F_Pin|LED_G_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, LED_E_Pin|LED_DP_Pin, GPIO_PIN_SET);
	}

	if(0 == pos){
		HAL_GPIO_WritePin(GPIOA, LED_1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, LED_2_Pin|LED_3_Pin|LED_4_Pin, GPIO_PIN_RESET);
	}else if(1 == pos){
		HAL_GPIO_WritePin(GPIOA, LED_2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, LED_1_Pin|LED_3_Pin|LED_4_Pin, GPIO_PIN_RESET);
	}else if(2 == pos){
		HAL_GPIO_WritePin(GPIOA, LED_3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, LED_1_Pin|LED_2_Pin|LED_4_Pin, GPIO_PIN_RESET);
	}else if(3 == pos){
		HAL_GPIO_WritePin(GPIOA, LED_4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, LED_1_Pin|LED_2_Pin|LED_3_Pin, GPIO_PIN_RESET);
	}

}

void Show_Digital_LED_2(uint8_t pin, uint8_t pos)
{
	uint16_t GPIO_Pin = ~pin;
	HAL_GPIO_WritePin(GPIOF, GPIO_Pin, GPIO_PIN_RESET);

	GPIO_Pin = pin;
	HAL_GPIO_WritePin(GPIOF, GPIO_Pin, GPIO_PIN_SET);

	if(0 == pos){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	}else if(1 == pos){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	}else if(2 == pos){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	}else if(3 == pos){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	}
}

/* USER CODE END 2 */
