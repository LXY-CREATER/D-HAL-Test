/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
int fputc(int ch,FILE*f)
{
	HAL_UART_Transmit(&huart1,(uint8_t*)&ch,1,0xffff);
	return ch;
}
int fgetc(FILE*f)
{
	uint8_t ch=0;
	HAL_UART_Receive(&huart1,&ch,1,0xffff);
	return ch;
}

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//int upEdge=0;
//int downEdge=0;
float distance=0;
char message[20]="";
uint8_t recever_data [1];
//void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
//		if(htim==&htim8&&htim->Channel==HAL_TIM_ACTIVE_CHANNEL_4){
//			 upEdge =HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_3);
//			 downEdge =HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_4);
//				//捕获后需手动清除中断标志，否则可能重复计数（中断清除函数）
//			 __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_CC3 | TIM_FLAG_CC4);
//				//处理溢出
//			 float edige;
//			 static uint32_t overflow_count = 0;
//			 if (downEdge < upEdge) {
//					 edige = (0xFFFF - upEdge) + downEdge + (overflow_count * 0x10000);
//				} else {
//					 edige = downEdge - upEdge;
//				}

//			 sprintf(message,"edige:%.2f   \r\n",edige);
//			 HAL_UART_Transmit(&huart1,(uint8_t *)message,sizeof(message),0xff);
//			 distance=(edige*0.034)/2;
//		 }
//}
// 初始化时启动上升沿捕获
/*void Inputcapture_Init(void) {
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
}

// 输入捕获中断回调
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    static uint32_t t1 = 0, t2 = 0;
    
    if (htim->Instance == TIM1) {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
            // 上升沿捕获
            t1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
            // 切换为下降沿捕获
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
        } else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
            // 下降沿捕获
            t2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
            // 计算时间差并转换为距离
            uint32_t delta = t2 - t1;
            distance = (delta * 0.034) / 2; // 单位：厘米
            // 切换回上升沿捕获，准备下一次测量
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
        }
    }
}*/
//// 启动输入捕获中断
//HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_4);

// 输入捕获中断回调函数
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM8) {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {
            // 处理上升沿/下降沿捕获
            static uint32_t rise_time = 0, fall_time = 0;
            if (__HAL_TIM_GET_CAPTUREPOLARITY(htim, TIM_CHANNEL_4) == TIM_INPUTCHANNELPOLARITY_RISING) {
                rise_time = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_4, TIM_INPUTCHANNELPOLARITY_FALLING);
            } else {
                fall_time = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
                uint32_t pulse_width = fall_time - rise_time;
                distance = (pulse_width * 0.034) / 2; // 计算距离（单位：cm）
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_4, TIM_INPUTCHANNELPOLARITY_RISING);
            }
        }
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */
HAL_TIM_Base_Start(&htim8);
HAL_TIM_IC_Start_IT(&htim8,TIM_CHANNEL_3);
HAL_TIM_IC_Start_IT(&htim8,TIM_CHANNEL_4);
 
char message[20]="";
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//		HAL_GPIO_WritePin(Trig_GPIO_Port,Trig_Pin,GPIO_PIN_SET);
//		HAL_Delay(1);
//		HAL_GPIO_WritePin(Trig_GPIO_Port,Trig_Pin,GPIO_PIN_RESET);
//		__HAL_TIM_SET_COUNTER(&htim8,0);
//		HAL_Delay(20);
		// 触发超声波模块
HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_SET);  // PE2 置高
delay_us(10);                                        // 维持 10μs
HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET); // PE2 置低

		sprintf(message,"distanse:%.2f   \r\n",distance);
		HAL_UART_Transmit(&huart1,(uint8_t *)message,sizeof(message),0xff);
		HAL_UART_Receive_IT(&huart1,recever_data,1);
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
