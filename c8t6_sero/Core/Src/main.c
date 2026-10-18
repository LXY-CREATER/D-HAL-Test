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

uint16_t TIM3_pwmVal=0;     //TIM3的占空比
uint16_t TIM1_pwmVal=0;     //TIM3的占空比
uint16_t TIM3_direction=1;  //控制TIM3的呼吸；1是越来越亮，0是越来越暗
uint16_t TIM1_direction=1;  //控制TIM4的呼吸；1是越来越亮，0是越来越暗

int i;
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

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

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
//  /* USER CODE BEGIN 1 */
//   // 1.启动舵机PWM（使用TIM1通道1）
// HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
//	//  2.设置舵机初始位置（0度）
// Servo_SetAngle(&htim1, TIM_CHANNEL_1, 0);
// Servo_Test(); 
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
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
			
//			Servo_Test(); 
//      Servo_SetAngle(&htim1, TIM_CHANNEL_1, 90); // 转向90度
//		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_4);  // PB4
//    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);  // PB5
//    HAL_Delay(500);  // 500ms方波
//		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 500);  // 50%占空比
//__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 500);
//		HAL_Delay(50);  //控制呼吸灯呼吸速率
//                   //控制占空比大小，即修改CCRx的大小
//        if(TIM3_direction){
//            TIM3_pwmVal=TIM3_pwmVal+10;
//            
//        }else{
//            TIM3_pwmVal=TIM3_pwmVal-10;
//        }
//        //设置了ARR为499，因此每计数500为一个周期
//        if(TIM3_pwmVal > 500){ 
//            TIM3_direction = 0;  //改变呼吸灯方向
//        }else if(TIM3_pwmVal == 0){
//            TIM3_direction = 1;  //改变呼吸灯方向
//        }
//        //修改定时器3通道2的PWM的占空比
//        __HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_2,TIM3_pwmVal);
//          if(TIM1_direction){
//            TIM1_pwmVal=TIM1_pwmVal+20;
//            
//        }else{
//            TIM1_pwmVal=TIM1_pwmVal-20;
//        }
//        if(TIM1_pwmVal > 500){ 
//            TIM1_direction = 0;  //改变呼吸灯方向
//        }else if(TIM1_pwmVal == 0){
//            TIM1_direction = 1;  //改变呼吸灯方向
//        }
//        //修改定时器4通道2的PWM的占空比
//        __HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_2,TIM1_pwmVal);

for(i=500;i<2400;i+=100)
	{
		__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,i);
		HAL_Delay(100);
	}
	for(i=2500;i>600;i-=100)
	{
		__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,i);
		HAL_Delay(100);
	}
		
		 HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_4);  // PB4
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);  // PB5
    HAL_Delay(500);  // 500ms方波

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
