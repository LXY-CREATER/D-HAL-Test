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
#include "sero.h"
#include "inputcapture.h"
#include "uart.h"
#include "motor.h"
#include "control.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
	

	// ���ϲ���
#define OBSTACLE_DISTANCE 30.0f  // �ϰ�������루���ף�
#define AVOIDANCE_DELAY 300      // ���϶�������ʱ�䣨���룩
#define BACKWARD_DELAY 500       // ������ʱ(����)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// ȫ�ֱ�������
volatile uint32_t last_tick = 0;
uint32_t avoidance_start_time = 0;
uint8_t is_avoiding = 0;
//OperationMode current_mode = MODE_MANUAL;

// ����UART����������
extern UART_Handler bluetooth_uart; 
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Debug_MovementTest(void);
void AutoAvoidance(void);
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
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	
	//1.��ʼ������
		HAL_TIM_Base_Start_IT(&htim1);   // ��������ʱ��
 /* ����˵����&bluetooth_uart��Ҫ��ʼ���Ľṹ�壩��&huart1������ʹ�õ�UART�������NULL�����ջص�����������չ��*/
		UART_Init(&bluetooth_uart, &huart1, Bluetooth_CommandCallback); 
// �ڳ�ʼ����ɺ�����
HAL_Delay(1000);
UART_SendString(&bluetooth_uart, "System Ready - Bluetooth Test\n");
 //2.�������PWM��ʹ��TIM2��TIM3��	
		HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  //�����ʼ��
		Motor_Init();
 //3.�������PWM��ʹ��TIM4ͨ��3��
		HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
// //  ���ö����ʼλ�ã�0�ȣ�
//	  Servo_SetAngle(&htim4, TIM_CHANNEL_3, 0);
//	  Servo_Test(); 
  // 4.�������Ƴ�ʼ��
    Bluetooth_Control_Init();
    // ��ѡ������һ�ε��Բ���
    // Debug_MovementTest();
 // 5. ������ģ���ʼ����ִֻ��һ�Σ���
		Inputcapture_Init();
  
 // 6. ���õ����ʼ״̬��ֹͣ��
		//Move_Stop();
// ���Դ��루�ɼ���main�����У�
Motor_Init(); // ȷ�������ʼ��ִ��
 // 7. ��ʼLED״̬
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
  
 // �����ɹ���ʾ
  char init_msg[] = "System Initialized\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)init_msg, strlen(init_msg), 100);
///*��������    ����������ת*/
//char test_msg[] = "Send motor commands (F/B/L/R/Q/E/S) to test Bluetooth\r\n";
//HAL_UART_Transmit(&huart1, (uint8_t*)test_msg, strlen(test_msg), 100);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
        UART_Process(&bluetooth_uart);
        
        Race_Process();
        
        if (GetOperationMode() == MODE_AUTO) {
            AutoAvoidance();
        } else if (GetOperationMode() == MODE_MANUAL) {
            float dist = Inputcapture_GetDistance();
            static uint32_t last_dist_time = 0;
            if (HAL_GetTick() - last_dist_time > 1000) {
                last_dist_time = HAL_GetTick();
                char message[50];
                sprintf(message, "Distance: %.2f cm (Manual Mode)\r\n", dist);
                HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), 100);
            }
        }
		
//		   // �ڲ���ģʽ�£���ִ��ʵ�ʵı����߼�
//    if (GetOperationMode() == MODE_AUTO) {
//        // �Զ�ģʽģ��
//        static uint32_t last_auto_msg = 0;
//        if (HAL_GetTick() - last_auto_msg > 2000) {
//            last_auto_msg = HAL_GetTick();
//            UART_SendString(&bluetooth_uart, "[SIMULATION] Auto mode: Scanning for obstacles...\r\n");
//            
//            // ģ��������
//            float simulated_dist = 50.0 + (rand() % 100); // �������50-150cm
//            char dist_msg[50];
//            sprintf(dist_msg, "[SIMULATION] Distance: %.1f cm\r\n", simulated_dist);
//            UART_SendString(&bluetooth_uart, dist_msg);
//        }
//    }
		
		
    //  LED��˸ָʾϵͳ���У���������ʽ��
    if (HAL_GetTick() - last_tick >= 200) {
        last_tick = HAL_GetTick();
        HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
    }
    
    // 5. ����ʱ���������Ƶ���Ĳ���
    HAL_Delay(50);
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
// �Զ����Ϻ���
void AutoAvoidance(void)
{
    static uint32_t last_avoid_check = 0;
    uint32_t current_time = HAL_GetTick();
    
    // ÿ100ms���һ���ϰ���
    if (current_time - last_avoid_check >= 100) {
        last_avoid_check = current_time;
        
        // 1. ���������
        float dist = Inputcapture_GetDistance();
        
        // 2. ���;�����Ϣ
        char message[50];
        sprintf(message, "Distance: %.2f cm (Auto Mode)\r\n", dist);
        HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), 100);
        
        // 3. �����߼�
        if (is_avoiding) {
            // ����ִ�б��϶���
            if (current_time - avoidance_start_time >= AVOIDANCE_DELAY) {
                is_avoiding = 0;
                Move_Front(200); // ������ɺ����ǰ��
                HAL_UART_Transmit(&huart1, (uint8_t*)"Avoidance completed, continuing forward\r\n", 45, 100);
            }
        } 
        else if (dist < OBSTACLE_DISTANCE && dist > 0) {
//            // ��⵽�ϰ���
//            char avoid_msg[80];
//            sprintf(avoid_msg, "[AVOID] Triggered! Dist=%.2fcm < %.2fcm\r\n", dist, OBSTACLE_DISTANCE);
//            HAL_UART_Transmit(&huart1, (uint8_t*)avoid_msg, strlen(avoid_msg), 100);
//            
//            Move_Stop();
//            Move_RightFront(200); // ��ǰ����
//            avoidance_start_time = current_time;
//            is_avoiding = 1;
//            
//            HAL_UART_Transmit(&huart1, (uint8_t*)"Avoiding obstacle to right front\r\n", 34, 100);
//        } 
//        else {
//            // ���ϰ��ֱ��
//            if (!is_avoiding) {
//                Move_Front(200);
//            }
					
					
					// ��⵽�ϰ��ֹͣ->����->��ת->��ʱ
            is_avoiding = 1;
            avoidance_start_time = current_time;
            
            Move_Stop();
            HAL_UART_Transmit(&huart1, (uint8_t*)"Obstacle detected! Stopping...\r\n", 33, 100);
            HAL_Delay(300);
            
            Move_Back(150); // ����
            HAL_UART_Transmit(&huart1, (uint8_t*)"Moving backward...\r\n", 21, 100);
            HAL_Delay(BACKWARD_DELAY);
            
            Move_Right(150); // ��ת
            HAL_UART_Transmit(&huart1, (uint8_t*)"Turning right...\r\n", 19, 100);
        }
        else if (dist > 0) {
            // ���ϰ������ǰ��
            Move_Front(200);
        }
        else {
            // ������ֹͣ
            Move_Stop();
            HAL_UART_Transmit(&huart1, (uint8_t*)"Distance measurement error! Stopping\r\n", 40, 100);
           }
        }
    }


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
