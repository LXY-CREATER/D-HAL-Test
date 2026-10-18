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
#include "can.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
union{
	float speed;
	uint8_t speed_data[4];
}speed_def;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define MOTOR_ID 0x0201
//使能

void Start_Can(uint64_t start){
	 CAN_TxHeaderTypeDef txStart;
	uint8_t start_data[8];
	uint32_t mailbox;

//	start_data[0] = MOTOR_ID >> 8;
//	start_data[1] = MOTOR_ID & 0XFF;
	start_data[0]=0xFF;
	start_data[1]=0xFF;
	start_data[2]=0xFF;
	start_data[3]=0xFF;
	start_data[4]=0xFF;
	start_data[5]=0xFF;
	start_data[6]=0xFF;
	start_data[7]=0xFC;
	
	txStart.StdId= 1;
	txStart.IDE=CAN_ID_STD;
	txStart.RTR=CAN_RTR_DATA;
	txStart.DLC=8;
	
	HAL_CAN_AddTxMessage(&hcan,&txStart,start_data,&mailbox);
}
//速度模式下指令发送
void Can_SendSpeed(float speed){
	speed_def.speed = speed;
//	speed_def.speed_data就是数组本身
	CAN_TxHeaderTypeDef txHeader;
	uint8_t Data[6]={0};
	uint32_t mailbox;
	
	//构造速度指令
  Data[0]=(uint8_t)(speed_def.speed_data[0]);
	Data[1]=(uint8_t)(speed_def.speed_data[1]);
	Data[2]=(uint8_t)(speed_def.speed_data[2]);
	Data[3]=(uint8_t)(speed_def.speed_data[3]);
	
	txHeader.StdId= MOTOR_ID;
	txHeader.IDE=CAN_ID_STD;
	txHeader.RTR=CAN_RTR_DATA;
	txHeader.DLC=4;
	
	HAL_CAN_AddTxMessage(&hcan,&txHeader,Data,&mailbox);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	//接受过滤器配置
CAN_FilterTypeDef filter;
	
	filter.FilterIdHigh=0x0000;
	filter.FilterIdLow=0x0000;
	filter.FilterMaskIdHigh=0x0000;
	filter.FilterMaskIdLow=0x0000;
	filter.FilterFIFOAssignment= CAN_FILTER_FIFO0;
	filter.FilterBank=0;
	filter.FilterMode=CAN_FILTERMODE_IDMASK;
	filter.FilterScale=CAN_FILTERSCALE_32BIT;
	filter.FilterActivation=ENABLE;
  
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
  MX_CAN_Init();
  /* USER CODE BEGIN 2 */
	
	HAL_CAN_Start(&hcan);
	HAL_CAN_ConfigFilter(&hcan,&filter);
	HAL_CAN_ActivateNotification(&hcan,CAN_IT_RX_FIFO0_MSG_PENDING);

		Start_Can(10);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		float speed=15;
		Can_SendSpeed(speed);
		HAL_Delay(2000);
		Can_SendSpeed(0);
		HAL_Delay(2000);
		

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
