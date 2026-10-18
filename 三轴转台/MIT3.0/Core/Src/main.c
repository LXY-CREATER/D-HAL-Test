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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f1xx_hal.h"
#include "motor_control.h"
#include "can_protocol.h"
#include "system_state.h"
#include <stdio.h>
#include <string.h>

// 全局系统状态
SystemStateMachine g_system = {
    .current_state = SYS_STATE_INIT,
    .enable_timeout = 0,
    .last_heartbeat = 0,
    .last_status_print = 0,
    .motor_enabled = {0, 0, 0}
};

// 重定向printf到串口
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 10);
    return ch;
}

// 系统初始化
void system_init(void) {
    // 添加调试点1
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    
    printf("==== System Initialization ====\n");
    
    // 添加调试点2
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
    
    // 初始化电机控制器
    printf("Initializing motor X...\n");
    motor_init(&motor_x, MOTOR_X, 0, 360);
    
    printf("Initializing motor Y...\n");
    motor_init(&motor_y, MOTOR_Y, -90, 90);
    
    printf("Initializing motor Z...\n");
    motor_init(&motor_z, MOTOR_Z, -180, 180);
    
    // 初始状态设置
    g_system.current_state = SYS_STATE_INIT;
    g_system.enable_retry_count = 0;
    
    // 添加调试点3
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
    printf("Motors initialized. Starting state machine...\n");
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
}
// 系统状态机实现
void system_state_machine(void) {
    uint32_t now = HAL_GetTick();
    static SystemState prev_state = SYS_STATE_INIT;
    
    // 打印状态变化
    if(g_system.current_state != prev_state) {
        const char *state_names[] = {
            "INIT", "ENABLING_MOTORS", "RUNNING", "EMERGENCY_STOP"
        };
        printf("State changed: %s -> %s\n", 
               state_names[prev_state], 
               state_names[g_system.current_state]);
        prev_state = g_system.current_state;
    }
    
    // 每秒打印系统状态
    if(now - g_system.last_status_print > 1000) {
        g_system.last_status_print = now;
        printf("System State: %d, Motors: X:%d Y:%d Z:%d\n", 
               g_system.current_state,
               g_system.motor_enabled[0],
               g_system.motor_enabled[1],
               g_system.motor_enabled[2]);
    }

    switch(g_system.current_state) {
        case SYS_STATE_INIT:
            // 进入电机使能状态
            g_system.current_state = SYS_STATE_ENABLING_MOTORS;
            g_system.enable_timeout = now + 3000; // 3秒超时
            g_system.enable_retry_count = 0;
            printf("Enabling motors...\n");
            break;
            
        case SYS_STATE_ENABLING_MOTORS:
            // 每500ms发送使能命令
            if(now - g_system.last_enable_sent > 500) {
                g_system.last_enable_sent = now;
                g_system.enable_retry_count++;
                
                printf("Sending enable commands (retry %d)\n", g_system.enable_retry_count);
                
                // 发送使能命令
                if(!g_system.motor_enabled[0]) send_motor_enable(MOTOR_X, 1);
                if(!g_system.motor_enabled[1]) send_motor_enable(MOTOR_Y, 1);
                if(!g_system.motor_enabled[2]) send_motor_enable(MOTOR_Z, 1);
            }
            
            // 检查所有电机是否使能
            if(g_system.motor_enabled[0] && 
               g_system.motor_enabled[1] && 
               g_system.motor_enabled[2]) {
                g_system.current_state = SYS_STATE_RUNNING;
                HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET); // 亮灯
                printf("All motors enabled! Entering RUN state.\n");
            }
            // 检查超时或重试次数
            else if(now > g_system.enable_timeout || g_system.enable_retry_count > 5) {
                if(g_system.enable_retry_count > 5) {
                    printf("ERROR: Motor enable failed after %d retries\n", g_system.enable_retry_count);
                } else {
                    printf("ERROR: Motor enable timeout\n");
                }
                g_system.current_state = SYS_STATE_EMERGENCY_STOP;
            }
            break;
            
        case SYS_STATE_RUNNING:
            // 每500ms发送心跳包
            if(now - g_system.last_heartbeat > 500) {
                g_system.last_heartbeat = now;
                send_heartbeat();
            }
            break;
            
        case SYS_STATE_EMERGENCY_STOP:
            // 失能所有电机
            if(g_system.motor_enabled[0] || 
               g_system.motor_enabled[1] || 
               g_system.motor_enabled[2]) {
                printf("Emergency stop! Disabling motors...\n");
                send_motor_enable(MOTOR_X, 0);
                send_motor_enable(MOTOR_Y, 0);
                send_motor_enable(MOTOR_Z, 0);
                memset(g_system.motor_enabled, 0, sizeof(g_system.motor_enabled));
                HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET); // 灭灯
            }
            break;
    }
}

//// 紧急停止按钮处理
//void emergency_button_handler(void) {
//    static uint32_t last_press = 0;
//    uint32_t now = HAL_GetTick();
    
//    // 防抖处理
//    if(now - last_press > 200) {
//        if(HAL_GPIO_ReadPin(EMERGENCY_BTN_GPIO_Port, EMERGENCY_BTN_Pin) == GPIO_PIN_RESET) {
//            g_system.current_state = SYS_STATE_EMERGENCY_STOP;
//            last_press = now;
//            printf("Emergency button pressed!\n");
//        }
//    }
//}

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
  MX_CAN_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
//  // 先初始化串口再调用printf
//printf("Starting system initialization...\n");
	
	// 系统初始化
  system_init();
  
  // 启动CAN和定时器
  if(HAL_CAN_Start(&hcan) != HAL_OK) 
//		{
//    printf("CAN start failed!\n");
//  }
  
  // 启用CAN接收中断
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
  
  // 启动TIM1更新中断（1ms周期）
  if(HAL_TIM_Base_Start_IT(&htim1) != HAL_OK) 
//		{
//    printf("TIM1 start failed!\n");
//  }
  
//  printf("System started successfully!\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // 处理状态机
    system_state_machine();
    
    // 检查紧急停止按钮
    //emergency_button_handler();
    
    // 低功耗模式
    HAL_Delay(10);
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
	RCC_OscInitStruct.HSIState = RCC_HSI_ON; 
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

// TIM1中断处理 (1ms周期)
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if(htim->Instance == TIM1) {
    motor_control_update(&motor_x);
    motor_control_update(&motor_y);
    motor_control_update(&motor_z);
  }
}

// CAN接收中断
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  CAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];
  
  if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
    can_receive_handler(rx_header.StdId, rx_data);
  }
}

// CAN错误回调
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {
  printf("CAN error: 0x%08lX\n", hcan->ErrorCode);
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
  printf("Fatal error! System halted.\n");
  while (1)
  {
    // 错误指示灯闪烁
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    HAL_Delay(200);
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
  printf("Assert failed: %s, line %lu\n", file, line);
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
