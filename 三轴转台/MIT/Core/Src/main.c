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
/*初始化*/

//电机参数
#define P_MIN   -12.5f     // 最小位置 (rad)
#define P_MAX   12.5f      // 最大位置 (rad)
#define V_MIN   -45.0f     // 最小速度 (rad/s)
#define V_MAX   45.0f      // 最大速度 (rad/s)
#define KP_MIN  0.0f       // 最小刚度系数
#define KP_MAX  500.0f     // 最大刚度系数
#define KD_MIN  0.0f       // 最小阻尼系数
#define KD_MAX  100.0f     // 最大阻尼系数
#define T_MIN   -18.0f     // 最小扭矩 (N·m)
#define T_MAX   18.0f      // 最大扭矩 (N·m)

/* 电机ID定义 */
#define MOTOR_ID 0x01      // 单个电机ID
#define FEEDBACK_ID (MOTOR_ID + 0x100) // MIT电机反馈ID = 电机ID + 0x100

/* 电机状态结构体 */
typedef struct {
    float position;      // 当前位置 (rad)
    float velocity;      // 当前速度 (rad/s)
    float torque;        // 当前扭矩 (N·m)
    uint32_t last_update; // 最后更新时间戳
    uint8_t enabled;     // 使能状态
} MotorState;

MotorState motor = {0};
uint32_t FlashTimer = 0;
uint32_t last_control_time = 0;

//使能代码
void Start_Motor(CAN_HandleTypeDef* hcan){
    CAN_TxHeaderTypeDef txHeader;
    uint8_t start_data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC};
    uint32_t mailbox;

    txHeader.StdId = MOTOR_ID;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.DLC = 8;
    
    HAL_CAN_AddTxMessage(hcan, &txHeader, start_data, &mailbox);
}

// 禁用电机
//void Stop_Motor(CAN_HandleTypeDef* hcan){
//    CAN_TxHeaderTypeDef txHeader;
//    uint8_t stop_data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD};
//    uint32_t mailbox;

//    txHeader.StdId = MOTOR_ID;
//    txHeader.IDE = CAN_ID_STD;
//    txHeader.RTR = CAN_RTR_DATA;
//    txHeader.DLC = 8;
//    
//    HAL_CAN_AddTxMessage(hcan, &txHeader, stop_data, &mailbox);
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
CAN_FilterTypeDef canFilter;
CAN_RxHeaderTypeDef rxHeader;
uint8_t rxData[8];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

// 函数声明
void enable_motor(CAN_HandleTypeDef* hcan);
void disable_motor(CAN_HandleTypeDef* hcan);
void ctrl_motor(CAN_HandleTypeDef* hcan, uint16_t id, float pos, float vel, float KP, float KD, float torq);
int float_to_uint(float x, float x_min, float x_max, int bits);
float uint_to_float(int x_int, float x_min, float x_max, int bits);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

float uint_to_float(int x_int, float x_min, float x_max, int bits){
    float span = x_max - x_min;
    return (x_int * span) / ((1 << bits) - 1) + x_min;
}

int float_to_uint(float x, float x_min, float x_max, int bits) {
    float span = x_max - x_min;
    float offset = x_min;
    return (int)((x - offset) * ((1 << bits) - 1) / span);
}

//控制模式发送函数
void ctrl_motor(CAN_HandleTypeDef* hcan, uint16_t id, float _pos, float _vel, float _KP, float _KD, float _torq) {
    CAN_TxHeaderTypeDef txHeader;
    uint8_t data[8];
    uint32_t mailbox;

    uint16_t pos_tmp = float_to_uint(_pos, P_MIN, P_MAX, 16);
    uint16_t vel_tmp = float_to_uint(_vel, V_MIN, V_MAX, 12);
    uint16_t kp_tmp = float_to_uint(_KP, KP_MIN, KP_MAX, 12);
    uint16_t kd_tmp = float_to_uint(_KD, KD_MIN, KD_MAX, 12);
    uint16_t tor_tmp = float_to_uint(_torq, T_MIN, T_MAX, 12);

    txHeader.StdId = id;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.DLC = 8;
    
    data[0] = (pos_tmp >> 8) & 0xFF;
    data[1] = pos_tmp & 0xFF;
    data[2] = (vel_tmp >> 4) & 0xFF;
    data[3] = ((vel_tmp & 0xF) << 4) | ((kp_tmp >> 8) & 0x0F);
    data[4] = kp_tmp & 0xFF;
    data[5] = (kd_tmp >> 4) & 0xFF;
    data[6] = ((kd_tmp & 0x0F) << 4) | ((tor_tmp >> 8) & 0x0F);
    data[7] = tor_tmp & 0xFF;

    HAL_CAN_AddTxMessage(hcan, &txHeader, data, &mailbox);
}

//接收分析函数
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan) {
    uint16_t p_int, v_int, t_int;
    
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK) {
        // 闪烁LED指示接收
        if (HAL_GetTick() - FlashTimer > 500) {
            HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
            FlashTimer = HAL_GetTick();
        }
        
        // 只处理电机反馈ID
        if (rxHeader.StdId == FEEDBACK_ID) {
            // 正确解析MIT电机反馈数据
            p_int = (rxData[0] << 8) | rxData[1];
            v_int = (rxData[2] << 4) | (rxData[3] >> 4);
            t_int = (rxData[4] << 4) | (rxData[5] >> 4);
            
            motor.position = uint_to_float(p_int, P_MIN, P_MAX, 16);
            motor.velocity = uint_to_float(v_int, V_MIN, V_MAX, 12);
            motor.torque = uint_to_float(t_int, T_MIN, T_MAX, 12);
            motor.last_update = HAL_GetTick();
            
            // 收到反馈表示电机已使能
            motor.enabled = 1;
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
  MX_CAN_Init();
  /* USER CODE BEGIN 2 */
  
  // 正确配置CAN过滤器 - 只接收反馈ID
  canFilter.FilterBank = 0;
  canFilter.FilterMode = CAN_FILTERMODE_IDLIST;
  canFilter.FilterScale = CAN_FILTERSCALE_32BIT;
  canFilter.FilterIdHigh = (FEEDBACK_ID << 5) & 0xFFFF; // 标准ID高16位
  canFilter.FilterIdLow = 0; 
  canFilter.FilterMaskIdHigh = 0;
  canFilter.FilterMaskIdLow = 0;
  canFilter.FilterFIFOAssignment = CAN_RX_FIFO0;
  canFilter.FilterActivation = ENABLE;
  canFilter.SlaveStartFilterBank = 14;
  
  HAL_CAN_ConfigFilter(&hcan, &canFilter);
  
  // 启动CAN并启用接收中断
  HAL_CAN_Start(&hcan);
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
  
  // 使能电机
  Start_Motor(&hcan);
  motor.enabled = 0; // 初始状态为未使能，等待反馈确认

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    // 如果电机未使能，定期重发使能命令
//    if (!motor.enabled && (HAL_GetTick() % 1000 == 0)) {
//        Start_Motor(&hcan);
//    }
    
    // 控制周期为1ms
//     if (motor.enabled && (HAL_GetTick() - last_control_time >= 1)) {
//        last_control_time = HAL_GetTick();
        
        float target_pos = 0.0f;  // 目标位置1弧度
        float target_vel = 10.0f;  // 目标速度0 rad/s
        float KP = 0.0f;         // 比例增益
        float KD = 1.0f;          // 微分增益
        
        // 计算误差
        float pos_error = target_pos - motor.position;
        float vel_error = target_vel - motor.velocity;
        
        // 计算输出力矩 (PD控制器)
        float torque = KP * pos_error + KD * vel_error;
        
        // 发送控制命令 (限制在±18Nm)
        if (torque > T_MAX) torque = T_MAX;
        if (torque < T_MIN) torque = T_MIN;
        
        ctrl_motor(&hcan, MOTOR_ID, 
                   target_pos=0.0f, target_vel=10.0f,
                   KP=0.0f, KD=1.0f, torque);
  //  }
    
    // 添加延时防止过度发送
    HAL_Delay(1);
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
