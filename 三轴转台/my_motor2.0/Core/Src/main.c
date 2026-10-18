/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
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
#include "can.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "stdio.h"
#include "mpu6050.h"
#include <math.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
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
#define MAX_YAW_ANGLE 30.0f  // 最大yaw角度
#define YAW_SCALE_FACTOR (1.0f / MAX_YAW_ANGLE)  // yaw缩放因子

// MPU6050相关常量定义
#define ACCEL_FULL_SCALE_8G     0x10
#define GYRO_FULL_SCALE_1000DPS 0x18
#define ACC_SENSITIVITY_8G      4096.0f
#define GYRO_SENSITIVITY_1000DPS 32.8f
#define RAD_TO_DEG 57.2957795131f
#define DEG_TO_RAD 0.01745329251f

struct three_motor {
    uint16_t MOTOR_ID_x;
    uint16_t MOTOR_ID_y;
    uint16_t MOTOR_ID_z;
};

struct three_motor MT = {0x03, 0x02, 0x01};
#define FEEDBACK_ID_x (MT.MOTOR_ID_x + 0x100) // MIT电机反馈ID = 电机ID + 0x100
#define FEEDBACK_ID_y (MT.MOTOR_ID_y + 0x100)
#define FEEDBACK_ID_z (MT.MOTOR_ID_z + 0x100)

typedef struct {
    float position;      // 当前位置 (rad)
    float velocity;      // 当前速度 (rad/s)
} MotorState;

MotorState motor_x = {0};
MotorState motor_y = {0};
MotorState motor_z = {0};

float filtered_roll = 0;
float filtered_pitch = 0;
float filtered_yaw = 0;
float raw_target_z=0;

// MPU6050全局变量
static float gx_bias = 0;
static float gy_bias = 0;
static float gz_bias = 0;
static int16_t ax, ay, az, gx, gy, gz;
static float axg, ayg, azg;
static float gx_dps, gy_dps, gz_dps;
static float roll_acc, pitch_acc;

int fputc(int ch, FILE *f) {
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

void Start_Motor(CAN_HandleTypeDef* hcan, uint8_t id) {
    CAN_TxHeaderTypeDef txHeader;
    uint8_t start_data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC};
    uint32_t mailbox;
    txHeader.StdId = id;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.DLC = 8;  
    HAL_CAN_AddTxMessage(hcan, &txHeader, start_data, &mailbox);
}

void Stop_Motor(CAN_HandleTypeDef* hcan, uint8_t id) {
    CAN_TxHeaderTypeDef txHeader;
    uint8_t stop_data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD};
    uint32_t mailbox;
    txHeader.StdId = id;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.DLC = 8;    
    HAL_CAN_AddTxMessage(hcan, &txHeader, stop_data, &mailbox);
}
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
CAN_FilterTypeDef canFilter;
CAN_RxHeaderTypeDef rxHeader;
uint8_t rxData[8];
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
float uint_to_float(int x_int, float x_min, float x_max, int bits) {
    float span = x_max - x_min;
    return (x_int * span) / ((1 << bits) - 1) + x_min;
}

int float_to_uint(float x, float x_min, float x_max, int bits) {
    float span = x_max - x_min;
    float offset = x_min;
    return (int)((x - offset) * ((1 << bits) - 1) / span);
}

// 低通滤波器函数
float low_pass_filter(float new_value, float old_value, float alpha) {
    return alpha * new_value + (1.0f - alpha) * old_value;
}

// 位置控制模式发送函数
void ctrl_motor(CAN_HandleTypeDef* hcan, uint16_t id, float _pos, float _vel, float _KP, float _KD, float _torq) {
    CAN_TxHeaderTypeDef txHeader;
    uint8_t data[8];
    uint32_t mailbox;
    
    // 限制参数在允许范围内
    _pos = (_pos < P_MIN) ? P_MIN : ((_pos > P_MAX) ? P_MAX : _pos);
    _vel = (_vel < V_MIN) ? V_MIN : ((_vel > V_MAX) ? V_MAX : _vel);
    _KP = (_KP < KP_MIN) ? KP_MIN : ((_KP > KP_MAX) ? KP_MAX : _KP);
    _KD = (_KD < KD_MIN) ? KD_MIN : ((_KD > KD_MAX) ? KD_MAX : _KD);
    _torq = (_torq < T_MIN) ? T_MIN : ((_torq > T_MAX) ? T_MAX : _torq);

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

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan) {
    uint16_t v_x, v_y, v_z;
    
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK) {
        // 只处理电机反馈ID
        if (rxHeader.StdId == FEEDBACK_ID_x) {
            v_x = (rxData[2] << 4) | (rxData[3] >> 4);
            motor_x.velocity = uint_to_float(v_x, V_MIN, V_MAX, 12);
            motor_x.position = uint_to_float((rxData[0] << 8) | rxData[1], P_MIN, P_MAX, 16);
        }
        else if (rxHeader.StdId == FEEDBACK_ID_y) {
            v_y = (rxData[2] << 4) | (rxData[3] >> 4);
            motor_y.velocity = uint_to_float(v_y, V_MIN, V_MAX, 12);
            motor_y.position = uint_to_float((rxData[0] << 8) | rxData[1], P_MIN, P_MAX, 16);
        }
        else if (rxHeader.StdId == FEEDBACK_ID_z) {
            v_z = (rxData[2] << 4) | (rxData[3] >> 4);
            motor_z.velocity = uint_to_float(v_z, V_MIN, V_MAX, 12);
            motor_z.position = uint_to_float((rxData[0] << 8) | rxData[1], P_MIN, P_MAX, 16);
        }
    }
}






void computer() {
    static uint32_t prev_time = 0;
    uint32_t current_time = HAL_GetTick();
    float dt = (current_time - prev_time) / 1000.0f;
    prev_time = current_time;
    
    // 确保时间间隔合理
    if (dt <= 0 || dt > 0.1f) dt = 0.01f;
    
    // 读取传感器数据
    MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);
    
    // 转换为物理单位
    axg = ax / ACC_SENSITIVITY_8G;
    ayg = ay / ACC_SENSITIVITY_8G;
    azg = az / ACC_SENSITIVITY_8G;
    
    gx_dps = (gx - gx_bias) / GYRO_SENSITIVITY_1000DPS;
    gy_dps = (gy - gy_bias) / GYRO_SENSITIVITY_1000DPS;
    gz_dps = (gz - gz_bias) / GYRO_SENSITIVITY_1000DPS;
    
    // 计算加速度计姿态
    roll_acc = atan2f(ayg, azg) * RAD_TO_DEG;
    pitch_acc = atan2f(-axg, sqrtf(ayg*ayg + azg*azg)) * RAD_TO_DEG;
    
    // 互补滤波更新姿态
    static float last_roll = 0, last_pitch = 0, last_yaw = 0;
    const float alpha = 0.98f;
    
    // Roll和Pitch使用加速度计校正
    float new_roll = alpha * (last_roll + gx_dps * dt) + (1 - alpha) * roll_acc;
    float new_pitch = alpha * (last_pitch + gy_dps * dt) + (1 - alpha) * pitch_acc;
    
    // Yaw直接使用陀螺仪积分 (加速度计无法校正Yaw)
    float new_yaw = last_yaw + gz_dps * dt;
    
    // 更新全局变量
    filtered_roll = new_roll;
    filtered_pitch = new_pitch;
    filtered_yaw = new_yaw;
    
    // 保存当前值供下次使用
    last_roll = new_roll;
    last_pitch = new_pitch;
    last_yaw = new_yaw;
    
    // 角度规范化到[-180, 180]范围
    filtered_roll = fmodf(filtered_roll + 180.0f, 360.0f);
    if (filtered_roll < 0) filtered_roll += 360.0f;
    filtered_roll -= 180.0f;
    
    filtered_pitch = fmodf(filtered_pitch + 180.0f, 360.0f);
    if (filtered_pitch < 0) filtered_pitch += 360.0f;
    filtered_pitch -= 180.0f;
    
    filtered_yaw = fmodf(filtered_yaw + 180.0f, 360.0f);
    if (filtered_yaw < 0) filtered_yaw += 360.0f;
    filtered_yaw -= 180.0f;
    
    // 调试输出
    static uint32_t last_print = 0;
    if (HAL_GetTick() - last_print > 100) {
        printf("Roll: %.1f°, Pitch: %.1f°, Yaw: %.1f°\n", 
               filtered_roll, filtered_pitch, filtered_yaw);
        printf("Gyro: X=%.1f, Y=%.1f, Z=%.1f dps\n", gx_dps, gy_dps, gz_dps);
        last_print = HAL_GetTick();
    }
}

// 修改后的定时器中断回调 - 位置控制模式
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    const float filter_alpha = 0.3f; // IMU低通滤波系数
    const float target_filter_alpha = 0.05f; // 目标位置滤波系数
    
    if (htim->Instance == TIM3) {
        static float last_target_z = 0; // 保存上一次的目标位置    
        static float last_yaw_value = 0;
        
        // 读取MPU6050数据
        computer(); // 更新全局变量filtered_roll, filtered_pitch, filtered_yaw
        
        // 计算目标位置 (限制在±1.0弧度内)
        float target_x = filtered_roll * YAW_SCALE_FACTOR;
        float target_y = filtered_pitch * YAW_SCALE_FACTOR;
        raw_target_z = filtered_yaw * YAW_SCALE_FACTOR*0.01;
        //float target_z = filtered_yaw * 0.04;
			
 // === 关键修改1: 反转yaw轴方向 ===
        raw_target_z = -raw_target_z;  // 添加负号反
			
        // 对目标位置进行低通滤波，使变化更平滑
        float target_z = low_pass_filter(raw_target_z, last_target_z, target_filter_alpha);
        last_target_z = target_z;
        
			// === 关键修改2: 限制位置范围 ===
        target_z = fmaxf(fminf(target_z, 1.0f), -1.0f);  // 限制在±1.0rad内
			
        // 计算速度前馈（减少起步震动）
        float yaw_velocity = (filtered_yaw - last_yaw_value) * 0.01f; // 10 = 100Hz * 0.1
        last_yaw_value = filtered_yaw;
        
        // 限制速度范围
        yaw_velocity = fmaxf(fminf(yaw_velocity, 5.0f), -5.0f);
        float velocity_feedforward = yaw_velocity * 0.1f; // 速度前馈系数
				
        // 发送位置控制命令
        ctrl_motor(&hcan, MT.MOTOR_ID_x, target_x, 0, 5.0, 2.0, 0);
        ctrl_motor(&hcan, MT.MOTOR_ID_y, target_y, 0, 5.0, 9.0, 0);
        ctrl_motor(&hcan, MT.MOTOR_ID_z, target_z,velocity_feedforward , 2.0f, 10.0f, 0);
        
        // 日志打印
        static uint32_t log_counter = 0;
        if (++log_counter >= 10) {
            log_counter = 0;
            printf("Yaw: %.1f°->%.3frad, VelFF: %.3f\r\n", 
                   filtered_yaw, target_z,MT.MOTOR_ID_z);
        }
    }
}
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
  MX_USART1_UART_Init();
  MX_I2C2_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
    // 配置CAN过滤器 - 允许所有消息通过
    canFilter.FilterBank = 0;
    canFilter.FilterMode = CAN_FILTERMODE_IDMASK;
    canFilter.FilterScale = CAN_FILTERSCALE_32BIT;
    canFilter.FilterIdHigh = 0x0000;
    canFilter.FilterIdLow = 0x0000;
    canFilter.FilterMaskIdHigh = 0x0000;
    canFilter.FilterMaskIdLow = 0x0000;
    canFilter.FilterFIFOAssignment = CAN_RX_FIFO0;
    canFilter.FilterActivation = ENABLE;
    HAL_CAN_ConfigFilter(&hcan, &canFilter);
  
    // 启动CAN并启用接收中断
    HAL_CAN_Start(&hcan);
    HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
    
    // 使能电机
    Start_Motor(&hcan, MT.MOTOR_ID_x);
    HAL_Delay(50);
    Start_Motor(&hcan, MT.MOTOR_ID_y);
    HAL_Delay(50);
    Start_Motor(&hcan, MT.MOTOR_ID_z);
    HAL_Delay(50);
    
    printf("System Initialized. Starting position control...\r\n");
    
    // 初始化MPU6050
    MPU6050_Init();
    Calibrate_Gyro(); // 校准陀螺仪
    
    // 启动定时器中断
    HAL_TIM_Base_Start_IT(&htim3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {        
    HAL_Delay(100);
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
