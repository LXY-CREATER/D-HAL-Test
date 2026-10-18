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
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "stdio.h"
#include "mpu6050.h"

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
struct three_motor
{
	uint16_t MOTOR_ID_x;
	uint16_t MOTOR_ID_y;
	uint16_t MOTOR_ID_z;
};
struct pid_module
{
	float kp;
	float ki;
  float kd;
  float integral;
  float prev_error;
	float max_output;
};
struct three_motor MT = {0x01, 0x02, 0x03};
struct pid_module pid={1.0f,0.0f,0.1,0.0f,0.0f,20.0f};
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
uint8_t x_flag=0;
uint8_t y_flag=0;
uint8_t z_flag=0;

uint8_t x_ctrl=0;
uint8_t y_ctrl=0;
uint8_t z_ctrl=0;
float dt=0.001f;
float vel_cmd_x=0;
float vel_cmd_y=0;
float vel_cmd_z=0;
int fputc(int ch, FILE *f) {
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
void Start_Motor(CAN_HandleTypeDef* hcan,uint8_t id)
{
    CAN_TxHeaderTypeDef txHeader;
    uint8_t start_data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC};
    uint32_t mailbox;
    txHeader.StdId = id;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.DLC = 8;  
    HAL_CAN_AddTxMessage(hcan, &txHeader, start_data, &mailbox);
}

// 禁用电机
void Stop_Motor(CAN_HandleTypeDef* hcan,uint8_t id)
{
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

float convet_pitch(int64_t pitch_x)
{
    if(pitch_x<450 && pitch_x>-450)
    {
         return (4.0f * pitch_x) / 450.0f;
    }
    else if ((pitch_x > -900 && pitch_x < -450) || (pitch_x > 450 && pitch_x < 900))
    {
        if(pitch_x<0)
         return -5.0f;
        else
         return 5.0f; 
    }
    else
    {
        Stop_Motor(&hcan,MT.MOTOR_ID_x);
        return 0;
    }
}
float convet_roll(int64_t roll_y)
{
    if(roll_y<900&&roll_y>900)
    {
         return (2.0f * roll_y) / 450.0f;
    }
    else if ((roll_y > -1800 && roll_y < -900) || (roll_y > 900 && roll_y < 1800))
    {
        if(roll_y<0)
         return -5.0f;
        else
         return 5.0f; 
    }
    else
    {
        Stop_Motor(&hcan,MT.MOTOR_ID_y);
        return 0;
    }
}
float convet_yaw(int64_t yaw_z)
{
    if(yaw_z<900&&yaw_z>-900)
    {
         return  (float)(yaw_z*4.0f) / 500.0f;
    }
    else if ((yaw_z> -1800 && yaw_z < -900) || (yaw_z > 900 && yaw_z < 1800))
    {
        if(yaw_z<0)
         return -5.0f;
        else
         return 5.0f; 
    }
    else
    {
        Stop_Motor(&hcan,MT.MOTOR_ID_z);
        return 0;
    }
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

void my_log()
{
	printf("x_velocity:%.1f", motor_x.velocity);
	printf("y_velocity:%.1f",motor_y.velocity);
	printf("z_velocity:%.1f",motor_z.velocity);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan) {
    uint16_t  v_x, v_y, v_z;
    
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK) {
        
        // 只处理电机反馈ID
        if (rxHeader.StdId == FEEDBACK_ID_x)
					{
            // 正确解析MIT电机反馈数据
            v_x = (rxData[2] << 4) | (rxData[3] >> 4);
            
            motor_x.velocity = uint_to_float(v_x, V_MIN, V_MAX, 12);
						
            x_flag=1;
        }
				 if (rxHeader.StdId == FEEDBACK_ID_y) 
					 {
            // 正确解析MIT电机反馈数据         
            v_y = (rxData[2] << 4) | (rxData[3] >> 4);
                      
            motor_y.velocity = uint_to_float(v_y, V_MIN, V_MAX, 12);
						 
						 y_flag=1;
        }
				  if (rxHeader.StdId == FEEDBACK_ID_z)
						{
            // 正确解析MIT电机反馈数据        
            v_z = (rxData[2] << 4) | (rxData[3] >> 4);						
            motor_z.velocity = uint_to_float(v_z, V_MIN, V_MAX, 12); 
							
						 z_flag=1;
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
 {
	 
     if (htim->Instance == TIM3) {
			 int pitch_int=pitch*10;
			 vel_cmd_x= convet_pitch(pitch_int);
			 x_ctrl = 1;
			 z_ctrl = 1;		
     }
 }
 
 void mit()
{
	ctrl_motor(&hcan,  MT.MOTOR_ID_x, 0, vel_cmd_x, 0, 1.0, 0);
	HAL_Delay(1);
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
  MX_I2C2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	canFilter.FilterBank = 0;
  canFilter.FilterMode = CAN_FILTERMODE_IDLIST;
	canFilter .FilterScale = CAN_FILTERSCALE_32BIT;
  canFilter.FilterIdHigh = (0x101 << 5);
  canFilter.FilterIdLow = (0x102 << 5);
  canFilter.FilterMaskIdHigh = (0x103 << 5);
  canFilter.FilterMaskIdLow = 0;
  canFilter.FilterActivation = ENABLE;

	HAL_CAN_ConfigFilter(&hcan, &canFilter);
  
  // 启动CAN并启用接收中断
  HAL_CAN_Start(&hcan);
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
	
  
  // 使能电机
	Start_Motor(&hcan,MT.MOTOR_ID_x);
	HAL_Delay(1);
  Start_Motor(&hcan,MT.MOTOR_ID_y);
	HAL_Delay(1);
	Start_Motor(&hcan,MT.MOTOR_ID_z);
	HAL_Delay(1);


	 HAL_TIM_Base_Start_IT(&htim3);
	
	 HAL_TIM_Base_Start_IT(&htim4);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		computer();
		if(x_ctrl==1&&z_ctrl==1)
		{
			x_ctrl=0;
			
			z_ctrl=0;
			mit();
		}
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
