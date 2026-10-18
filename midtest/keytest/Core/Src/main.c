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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

// 全局变量
extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart1;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// ADC读取方案
Keypad_Key_t KEYPAD_ReadADC(void)
{
    static uint32_t adc_samples[10] = {0};
    static uint8_t sample_index = 0;
    static Keypad_Key_t last_stable_key = KEY_NONE;
    
    uint32_t adc_value;
    Keypad_Key_t current_key = KEY_NONE;
    
    // 启动ADC转换
    HAL_ADC_Start(&hadc1);
    
    // 等待转换完成
    if(HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
    {
        adc_value = HAL_ADC_GetValue(&hadc1);
        
        // 存储采样值
        adc_samples[sample_index] = adc_value;
        sample_index = (sample_index + 1) % 10;
        
        // 计算平均值
        uint32_t sum = 0;
        for(int i = 0; i < 10; i++) {
            sum += adc_samples[i];
        }
        uint32_t average = sum / 10;
        
        // 根据ADC值判断按键
        // 注意：这些阈值需要根据实际键盘调整
        if(average < 100) current_key = KEY_NONE;
        else if(average < 300) current_key = KEY_1;
        else if(average < 550) current_key = KEY_2;
        else if(average < 800) current_key = KEY_3;
        else if(average < 1000) current_key = KEY_4;
        else if(average < 1200) current_key = KEY_5;
        else if(average < 1300) current_key = KEY_6;
        else if(average < 1450) current_key = KEY_7;
        else if(average < 1550) current_key = KEY_8;
        else if(average < 1650) current_key = KEY_9;
        else if(average < 1750) current_key = KEY_10;
        else if(average < 1900) current_key = KEY_11;
        else if(average < 2000) current_key = KEY_12;
        else current_key = KEY_INVALID;
        
        // 消抖处理
        static uint8_t stable_count = 0;
        static Keypad_Key_t last_key = KEY_NONE;
        
        if(current_key == last_key) {
            stable_count++;
            if(stable_count >= 5) { // 连续5次相同认为稳定
                last_stable_key = current_key;
                stable_count = 0;
            }
        } else {
            stable_count = 0;
            last_key = current_key;
        }
    }
    
    HAL_ADC_Stop(&hadc1);
    return last_stable_key;
}

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

// 数字信号读取方案
Keypad_Key_t KEYPAD_ReadDigital(void)
{
    static uint32_t signal_history[20] = {0};
    static uint8_t history_index = 0;
    static Keypad_Key_t last_key = KEY_NONE;
    
    // 读取当前信号状态
    uint32_t current_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    
    // 更新历史记录
    signal_history[history_index] = current_state;
    history_index = (history_index + 1) % 20;
    
    // 计算高电平比例
    uint32_t high_count = 0;
    for(int i = 0; i < 20; i++) {
        high_count += signal_history[i];
    }
    
    // 根据信号特征判断按键
    Keypad_Key_t detected_key = KEY_NONE;
    
    if(high_count == 0) {
        detected_key = KEY_NONE; // 持续低电平
    } else if(high_count == 20) {
        detected_key = KEY_12; // 持续高电平（假设为按键12）
    } else {
        // 根据脉冲模式判断具体按键
        // 这里需要根据实际键盘协议解码
        detected_key = (Keypad_Key_t)(high_count); // 简单映射
        if(detected_key > KEY_12) detected_key = KEY_INVALID;
    }
    
    // 按键释放检测
    static Keypad_Key_t pressed_key = KEY_NONE;
    
    if(detected_key != KEY_NONE && pressed_key == KEY_NONE) {
        pressed_key = detected_key;
        return detected_key;
    } else if(detected_key == KEY_NONE && pressed_key != KEY_NONE) {
        Keypad_Key_t temp = pressed_key;
        pressed_key = KEY_NONE;
        last_key = KEY_NONE;
        return KEY_NONE; // 返回NONE表示按键释放
    }
    
    last_key = detected_key;
    return KEY_NONE;
}

// 调试输出函数
void KEYPAD_DebugOutput(Keypad_Key_t key)
{
    char msg[50];
    
    switch(key) {
        case KEY_1: sprintf(msg, "Key 1 Pressed\r\n"); break;
        case KEY_2: sprintf(msg, "Key 2 Pressed\r\n"); break;
        case KEY_3: sprintf(msg, "Key 3 Pressed\r\n"); break;
        case KEY_4: sprintf(msg, "Key 4 Pressed\r\n"); break;
        case KEY_5: sprintf(msg, "Key 5 Pressed\r\n"); break;
        case KEY_6: sprintf(msg, "Key 6 Pressed\r\n"); break;
        case KEY_7: sprintf(msg, "Key 7 Pressed\r\n"); break;
        case KEY_8: sprintf(msg, "Key 8 Pressed\r\n"); break;
        case KEY_9: sprintf(msg, "Key 9 Pressed\r\n"); break;
        case KEY_10: sprintf(msg, "Key 10 Pressed\r\n"); break;
        case KEY_11: sprintf(msg, "Key 11 Pressed\r\n"); break;
        case KEY_12: sprintf(msg, "Key 12 Pressed\r\n"); break;
        case KEY_INVALID: sprintf(msg, "Invalid Key\r\n"); break;
        default: return; // KEY_NONE不输出
    }
    
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
}

// 信号分析函数（用于调试和校准）
void KEYPAD_AnalyzeSignal(void)
{
    static uint32_t last_print = 0;
    uint32_t current_time = HAL_GetTick();
    
    if(current_time - last_print > 500) { // 每500ms输出一次
        // ADC值
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 10);
        uint32_t adc_value = HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);
        
        // 数字信号
        uint32_t digital_value = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
        
        char analysis_msg[100];
        sprintf(analysis_msg, "ADC: %lu, Digital: %lu\r\n", adc_value, digital_value);
        HAL_UART_Transmit(&huart1, (uint8_t*)analysis_msg, strlen(analysis_msg), 100);
        
        last_print = current_time;
    }
}

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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
 // 校准ADC
    HAL_ADCEx_Calibration_Start(&hadc1);
    
    Keypad_Key_t last_detected_key = KEY_NONE;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		// 方案1：使用ADC读取（推荐先尝试此方案）
        Keypad_Key_t current_key = KEYPAD_ReadADC();
        
        // 方案2：使用数字信号读取（如果ADC方案不工作）
        // Keypad_Key_t current_key = KEYPAD_ReadDigital();
        
        if(current_key != KEY_NONE && current_key != last_detected_key) {
            KEYPAD_DebugOutput(current_key);
            
            // 控制LED指示（可选）
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        }
        
        last_detected_key = current_key;
        
        // 信号分析（调试用）
        KEYPAD_AnalyzeSignal();
        
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
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
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
