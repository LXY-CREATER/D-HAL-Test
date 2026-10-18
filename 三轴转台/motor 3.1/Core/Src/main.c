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

//// 全局变量
//extern MotorController motor_x, motor_y, motor_z;

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// 全局系统状态机定义
SystemStateMachine g_system = {
    .current_state = SYS_STATE_INIT,
    .enable_timeout = 0,
    .last_heartbeat = 0,
    .motor_enabled = {0, 0, 0}
};

// 系统初始化
void system_init(void) {
    // 初始化三个电机
    motor_init(&motor_x, MOTOR_X, 0, 360);   // X轴
    motor_init(&motor_y, MOTOR_Y, -90, 90);  // Y轴
    motor_init(&motor_z, MOTOR_Z, -180, 180);// Z轴
    
    g_system.current_state = SYS_STATE_INIT;
}

// 系统状态机实现
void system_state_machine(void) {
    static uint32_t last_enable_time = 0;
    uint32_t now = HAL_GetTick();
    
    switch(g_system.current_state) {
        case SYS_STATE_INIT:
            // 进入电机使能状态
            g_system.current_state = SYS_STATE_ENABLING_MOTORS;
            g_system.enable_timeout = now + 3000; // 3秒超时
            break;
            
        case SYS_STATE_ENABLING_MOTORS:
            // 每200ms发送使能命令
            if(now - last_enable_time > 200) {
                last_enable_time = now;
                
                // 发送使能命令
                send_motor_enable(MOTOR_X, 1);
                send_motor_enable(MOTOR_Y, 1);
                send_motor_enable(MOTOR_Z, 1);
                
                // 检查是否所有电机都已使能
                if(g_system.motor_enabled[0] && 
                   g_system.motor_enabled[1] && 
                   g_system.motor_enabled[2]) {
                    g_system.current_state = SYS_STATE_RUNNING;
                    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
                }
            }
            
            // 检查超时
            if(now > g_system.enable_timeout) {
                g_system.current_state = SYS_STATE_RUNNING;
                HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
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
            send_motor_enable(MOTOR_X, 0);
            send_motor_enable(MOTOR_Y, 0);
            send_motor_enable(MOTOR_Z, 0);
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
            break;
    }
}

// 紧急停止按钮处理
void emergency_button_handler(void) {
    // 假设紧急停止按钮接在PA0
    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
        g_system.current_state = SYS_STATE_EMERGENCY_STOP;
    }
}

// CAN接收处理
void can_receive_handler(uint32_t id, uint8_t* data) {
    // 处理电机反馈
    if(id == 0x204) { // X轴反馈
        g_system.motor_enabled[0] = data[0] & 0x01;
    } else if(id == 0x205) { // Y轴反馈
        g_system.motor_enabled[1] = data[0] & 0x01;
    } else if(id == 0x206) { // Z轴反馈
        g_system.motor_enabled[2] = data[0] & 0x01;
    }
}

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

// 添加串口重定向
//#include <stdio.h>

//#ifdef __GNUC__
//#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
//#else
//#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
//#endif

//PUTCHAR_PROTOTYPE {
//    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
//    return ch;
//}

// 添加简单的日志函数
//void log_message(const char *format, ...) {
//    char buffer[128];
//    va_list args;
//    va_start(args, format);
//    vsnprintf(buffer, sizeof(buffer), format, args);
//    va_end(args);
//    
//    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
//}

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
  MX_TIM1_Init();
  MX_TIM2_Init();
//  MX_TIM3_Init();
//  MX_TIM4_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

//// 初始化电机控制器
//  motor_init(&motor_x, MOTOR_X, 0, 360);  // 横滚轴
//  motor_init(&motor_y, MOTOR_Y, -90, 90); // 俯仰轴
//  motor_init(&motor_z, MOTOR_Z, -180, 180); // 偏航轴
  
  // 启动CAN和定时器
  HAL_CAN_Start(&hcan);
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
  HAL_TIM_Base_Start_IT(&htim1); //1ms定时器
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		  // 处理状态机
        system_state_machine();
        
        // 检查紧急停止按钮
        emergency_button_handler();
        
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
