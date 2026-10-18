/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sys.h"
#include "usart3.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
u8 Way_Angle=2;                             //???????????1???????  2????????  3????????? 
u8 Flag_front,Flag_back,Flag_Left,Flag_Right,Flag_velocity=2; //??????????????
u8 Flag_Stop=1,Flag_Show=0;                 //????????????????????  ????? ?????
int Motor_Left,Motor_Right;                 //???PWM???? ???Motor?? ??Moto???	
int Temperature;                            //??????
int Voltage,Middle_angle;                   //????????????????
float Angle_Balance,Gyro_Balance,Gyro_Turn; //?????? ????????? ?????????
u8 LD_Successful_Receive_flag;              //?????????????????
u8 Mode = Cooperative_Mode;				//????????????
u8 CCD_Zhongzhi,CCD_Yuzhi;                  //CCD????????
u16 ADV[128]={0};                           //???CCD???????????
u16 determine;                              //??????????????????
float Move_X,Move_Z;			            //??????????
u32 Distance;                               //?????????
u8 PID_Send; 					                    	//??????????
u8 Flag_follow=0,Flag_avoid=0;							//?????????????????????????
float Acceleration_Z;                       //Z???????  
volatile u8 delay_flag,delay_50;            //??????????
float Balance_Kp=25500,Balance_Kd=135,Velocity_Kp=16000,Velocity_Ki=80,Turn_Kp=4200,Turn_Kd=60;//PID?????????100??
float Yaw_Kp=360.0f,Yaw_Kd=24.0f;                    //?????PD????
int   Yaw_Limit=5000;                                  //??????????

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
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
	JTAG_Set(JTAG_SWD_DISABLE);     //???JTAG???
	JTAG_Set(SWD_ENABLE);           //??SWD??? ?????????????SWD??????
  delay_init();                   //???????????
  OLED_Init();                    //OLED?????
  MPU6050_initialize();           //MPU6050?????	
	DMP_Init();                     //?????DMP 
    while(Choose()) { }
	if(Mode==Ultrasonic_Avoid_Mode||Mode==Ultrasonic_Follow_Mode)
		  MX_TIM2_Init(); //???????????CCD??ELE???????????????CCD??ELE??????????????????????
	if(Mode==Lidar_Avoid_Mode||Mode==Lidar_Follow_Mode||Mode==Lidar_Straight_Mode)
		MX_USART2_UART_Init();
	if(Mode==CCD_Line_Patrol_Mode)
		CCD_Init();
	if(Mode==ELE_Line_Patrol_Mode)
		ELE_Init();	
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); //??????????????
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if(Flag_Show==0)          		//???MiniBalance APP??OLED?????
			{
//				Get_Angle(Way_Angle);   
				 APP_Show();								//?????????APP
				 oled_show();          			//???????
			}
			else                      		//???MiniBalance?????? ?????????????????????????????app???????OLED?????
			{
				 DataScope();          			//????MiniBalance??????
			}	
			//=====?????????200ms??????????????=====//
			if(Mode == Cooperative_Mode)
			{
				static u8 feedback_cnt = 0;
				feedback_cnt++;
				if(feedback_cnt >= 4) {        // 4*50ms = 200ms
					SendSlaveFeedback();
					feedback_cnt = 0;
				}
			}
		  delay_flag=1;	
		  delay_50=0;		
			while(delay_flag);	     			//????????50ms	??????????delay?????????????????MPU6050??????50ms???
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
