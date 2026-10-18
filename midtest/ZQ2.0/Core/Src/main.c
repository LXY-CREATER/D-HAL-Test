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
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "key.h"
#include "seg_display.h"
#include "servo.h"
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
SystemMode currentMode = INPUT_MODE;
uint16_t inputAngle = 0;         // 角度值（0-180）
uint8_t angleBuffer[3] = {0};    // 输入缓冲区（最多3位：180）
uint8_t bufferIndex = 0;         // 缓冲区索引
uint32_t confirmPressStart = 0;  // 确认键按下起始时间
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
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
System_Init();  // 初始化外设和模块
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		Mode_Handle();  // 处理当前模式逻辑
    HAL_Delay(10);  // 主循环延时去抖
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
// 系统初始化：初始化所有外设和模块
void System_Init(void) {
    // 初始化舵机（TIM1_CH1，PE9）
    Servo_Init(&htim1, TIM_CHANNEL_1);
    // 初始化数码管
    SegDisplay_Init();
    // 启动ADC（按键和电位器）
    HAL_ADC_Start(&hadc1);
    // 启动TIM3定时中断（用于数码管动态扫描，1ms一次）
    HAL_TIM_Base_Start_IT(&htim3);
}

// 模式处理主函数
void Mode_Handle(void) {
    KeyType key = Key_Scan();  // 读取按键

    switch (currentMode) {
        case INPUT_MODE:
            // 处理数字输入、确认、清除
            if (key != KEY_NONE) {
                if (key >= KEY_0 && key <= KEY_9) {
                    // 数字输入（限制3位，最大180）
                    if (bufferIndex < 3) {
                        angleBuffer[bufferIndex++] = key;
                        inputAngle = angleBuffer[0] * 100 + angleBuffer[1] * 10 + angleBuffer[2];
                        if (inputAngle > 180) inputAngle = 180;
                        SegDisplay_Update(inputAngle);  // 更新显示
                    }
                } else if (key == KEY_CLEAR) {
                    // 清除输入
                    bufferIndex = 0;
                    memset(angleBuffer, 0, sizeof(angleBuffer));
                    inputAngle = 0;
                    SegDisplay_Update(inputAngle);
                } else if (key == KEY_CONFIRM) {
                    // 短按：确认角度；长按5s：切换到电位器模式
                    confirmPressStart = HAL_GetTick();
                    while (Key_Scan() == KEY_CONFIRM) {  // 等待按键释放
                        if (HAL_GetTick() - confirmPressStart > 5000) {
                            currentMode = POTENTIOMETER_MODE;
                            return;
                        }
                    }
                    // 短按处理：闪烁2次后执行
                    SegDisplay_Blink(2);  // 闪烁2次
                    Servo_SetAngle(inputAngle);  // 舵机转动
                }
            }
            break;

        case POTENTIOMETER_MODE:
            // 电位器控制：实时读取并更新角度
            inputAngle = Potentiometer_GetAngle();  // 读取电位器角度（0-180）
            SegDisplay_Update(inputAngle);          // 更新显示
            Servo_SetAngle(inputAngle);             // 舵机随动

            // 长按确认键返回输入模式
            if (key == KEY_CONFIRM) {
                confirmPressStart = HAL_GetTick();
                while (Key_Scan() == KEY_CONFIRM) {
                    if (HAL_GetTick() - confirmPressStart > 5000) {
                        currentMode = INPUT_MODE;
                        // 保存当前角度到输入缓冲区
                        angleBuffer[0] = inputAngle / 100;
                        angleBuffer[1] = (inputAngle % 100) / 10;
                        angleBuffer[2] = inputAngle % 10;
                        bufferIndex = (inputAngle >= 100) ? 3 : (inputAngle >= 10 ? 2 : 1);
                        return;
                    }
                }
            }
            break;
    }
}

// 映射函数：将输入范围映射到输出范围
uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
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
