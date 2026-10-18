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
#include "usart.h"
#include "chaoshengbo.h"
#include "control.h"
#include "bluetooth.h"
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
 uint32_t speed = 500 ; // 500/2000= 25%的占空比，即速度为全速的1/4
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
  /* USER CODE BEGIN 1 */
extern  float distances;
uint32_t S=20;//设定距离标准值
uint32_t LS,RS=0;//测定的左右两边距离值
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
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

HAL_TIM_Base_Start_IT(&htim3);//开启定时器3中断
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); //直流电机PWM
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2); //SG90PWM
HAL_UART_Receive_IT(&huart2,(uint8_t *)&USART2_NewData,1); //开启启接收中断
SG90_Init();//SG90舵机初始化，也就是转90°，朝向小车前方
CAR_STOP();
HAL_Delay(500);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		//蓝牙控制变速
		if(USART2_RX_STA&0x8000)              //判断中断接收标志位（蓝牙模块使用USART2）
      {
    	 if((USART2_RX_STA&0x7FFF) ==4 	//判断接收数量是不是为四个
    			&& USART2_RX_BUF[0]==0xA5 	//判断接收第一个数据是不是包头0xA5
    			&& USART2_RX_BUF[3]==(USART2_RX_BUF[1]+USART2_RX_BUF[2])%0x100)	//判断接收校验码是不是原数据之和的低八位

    	{
    		 switch(USART2_RX_BUF[2])      //接收并读取蓝牙发送过来的第三个数,改变车速
    		 {   //TIM2计数周期ARR的值在CubeMX中设置为20000
    		 case(0x00):__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1,0);break;    //0%的速度

				 case(0x14):__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1,4000) ;break;//20%的速度

				 case(0x28):__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1,8000) ;break;//40%的速度

				 case(0x3C):__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1,12000);break;//60%的速度

				 case(0x50):__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1,16000);break;//80%的速度

				 case(0x64):__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1,20000);break;//100%的速度

				 default:break;

    		 }


    		 switch(USART2_RX_BUF[1])      //接收并读取蓝牙发送过来的第二个数，改变车的行驶状态
    		 {
				 case(0x00):CAR_STOP();break;
				 case(0x01):CAR_GO();break;
				 case(0x02):CAR_LGO();break;
				 case(0x03):CAR_RGO();break;
				 case(0x04):CAR_BACK();break;

				 default:break;

    		 }
    	 }
         USART2_RX_STA=0;//标志位清0，准备下次接收
      }
			
				if(distances>S)
	{
	    CAR_GO();
	}
		else
		{
				CAR_STOP();
				SG90_Turn_L();
				HAL_Delay(1500);
				LS=distances;
				SG90_Turn_R();
				HAL_Delay(1500);
				RS=distances;
				SG90_Init();
		
				if(LS>RS&&LS>S)
				{
					CAR_LGO();
					HAL_Delay(500);
				}
				else if(RS>LS&&RS>S)
				{
					CAR_RGO();
					HAL_Delay(500);
				}
				else
				{
					CAR_BACK();
					HAL_Delay(500);
				}
		}
  


  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */


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
