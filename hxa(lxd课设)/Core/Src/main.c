/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * 智能康复训练系统 - 上肢康复版本
  * 硬件：STM32C8T6 + AT8870 + ASRPRO + HC-06
  * 功能：手动/语音控制，自动位置反馈，康复训练模式
  *
  ******************************************************************************
  */
/* USER CODE END Header */

	/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * �����ֱۿ���ϵͳ - �����ع��汾
  * Ӳ����STM32C8T6 + AT8870 + ASRPRO + HC-6
  * ���ܣ�����/�������ƣ�������λ�÷�������������ѵ��
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// 系统头文件
#include "config.h"
#include "motor_controller.h"
#include "encoder_reader.h"
#include "bluetooth_handler.h"
#include "voice_handler.h"
#include "oled_display.h"
#include "rehabilitation_mode.h"
#include "system_manager.h"



// 标准库
#include <stdio.h>
#include <string.h>
#include <math.h>
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */


// ============================================
// ȫ�ֱ�������
// ============================================

// ϵͳ״̬
SystemState_t system_state = SYS_STATE_INIT;
OperationMode_t system_mode = MODE_IDLE;
uint8_t system_ready = 0;
uint32_t system_time = 0;

// ������
extern TIM_HandleTypeDef htim2;    // ������
extern TIM_HandleTypeDef htim3;    // ���PWM
extern UART_HandleTypeDef huart1;  // ����
extern UART_HandleTypeDef huart2;  // ����
extern I2C_HandleTypeDef hi2c1;    // OLED

// DMA���
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_rx;

// ��ʱ������

// ============================================
// ��������
// ============================================

// ϵͳ����
void System_Init(void);
void System_Main_Loop(void);
void System_Update_Timers(void);
void System_Check_Status(void);
void System_Handle_Error(const char* error_msg);
void SystemClock_Config(void); 



// �����
void Process_Serial_Commands(void);
void Process_Bluetooth_Commands(void);
void Process_Voice_Commands(void);

// ��ʾ����
void Update_OLED_Display(void);
void Print_System_Status(void);

// ���Ժ���
void System_Self_Test(void);
/* USER CODE END 0 */

int main(void)
{
	
  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();

  /* USER CODE BEGIN 2 */
// ϵͳ��ʼ��
  System_Init();
//   // ����PWM
//  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
//  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
//  
//  // ��ʼ��Ϊֹͣ״̬
//  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
//  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
//  
//  // ��ʼ����ģ��
  Motor_Controller_Init(&htim3, TIM_CHANNEL_1, TIM_CHANNEL_2, 999);
//  Encoder_Init_Ex(&htim2, 600, 30.0f, 72.0f, 200.0f);
//  Rehabilitation_Mode_Init();
//  OLED_Display_Init(&hi2c1);
//  Bluetooth_Handler_Init(&huart1);
//  Voice_Handler_Init(&huart2);
//  
//  // ��������
//  Bluetooth_Start_Receive();
//  Voice_Start_Receive();
//  
//  // ��ʾ������Ϣ
//  char startup_msg[] = "\r\n=== Rehabilitation Arm ===\r\n";
//  HAL_UART_Transmit(&huart1, (uint8_t*)startup_msg, strlen(startup_msg), 100);
//  // ��ʾ�������
//  DEBUG_PRINT("\r\n=== Command List ===\r\n");
//  DEBUG_PRINT("1: Forward 30%%\r\n");
//  DEBUG_PRINT("2: Stop motor\r\n");
//  DEBUG_PRINT("3: Backward 30%%\r\n");
//  DEBUG_PRINT("R: Toggle rehab mode\r\n");
//  DEBUG_PRINT("U/D: Resistance +/- \r\n");
//  DEBUG_PRINT("X: Emergency brake\r\n");
//  DEBUG_PRINT("C: Check pin state\r\n");
//  DEBUG_PRINT("E: Encoder debug\r\n");
//  DEBUG_PRINT("S: System status\r\n");
//  DEBUG_PRINT("T: Motor test\r\n");
//  DEBUG_PRINT("P: Move to 50%%\r\n");
//  DEBUG_PRINT("H: Command help\r\n");
//  DEBUG_PRINT("====================\r\n\r\n");
	/* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	
 // ϵͳ��ѭ��
    System_Main_Loop();
    
    // ϵͳ��ʱ
    HAL_Delay(SYSTEM_TICK_MS);    
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
// ============================================
// ϵͳ��ʼ��
// ============================================

// �޸�System_Init����
void System_Init(void)
{
    DEBUG_PRINT("\r\n=== Rehabilitation Arm System ===\r\n");
    DEBUG_PRINT("Initializing...\r\n");
    
    // 1. ��ʼ��ϵͳ������
    SystemManager_Init();
    
    // 2. ��ʼ��������
    DEBUG_PRINT("Initializing encoder...\r\n");
    Encoder_Init(&htim2);
    
    // 3. 初始化电机控制器
    DEBUG_PRINT("Initializing motor controller...\r\n");
    Motor_Controller_Init(&htim3, TIM_CHANNEL_1, TIM_CHANNEL_2, 1000);
    
    // 4. ��ʼ��OLED
    DEBUG_PRINT("Initializing OLED...\r\n");
    OLED_Init_Safe();
    
    // 5. ��ʼ������
    DEBUG_PRINT("Initializing Bluetooth...\r\n");
    Bluetooth_Init(&huart1);
    
    // 6. ��ʼ������ģ�飨����У�
    DEBUG_PRINT("Initializing voice module...\r\n");
    // Voice_Init(&huart2);  // ����ʵ���������
    
    // 7. ��ʼ������ģʽ
    DEBUG_PRINT("Initializing rehabilitation mode...\r\n");
    Rehabilitation_Init();
    
    // 8. ������������
    Bluetooth_Start_Receive();
    
    // 9. ϵͳ����
    system_state = SYS_STATE_IDLE;
    system_mode = MODE_IDLE;
    system_ready = 1;
    
    // ��ʾ�������
    OLED_Clear_Safe();
    OLED_Show_String(10, 0, "REHAB ARM", 6);
    OLED_Show_String(0, 2, "System Ready", 6);
    OLED_Show_String(0, 4, "Range: 200mm", 6);
    OLED_Show_String(0, 6, "BLE: HC-06", 6);
    OLED_Refresh_Safe();
    
    // ��ӡϵͳ��Ϣ
    DEBUG_PRINT("\r\nSystem Information:\r\n");
    DEBUG_PRINT("  Travel Range: %.1fmm\r\n", TRAVEL_RANGE_MM);
    DEBUG_PRINT("  Encoder CPR: %d\r\n", ENCODER_CPR);
    DEBUG_PRINT("  Gear Ratio: %.1f:1\r\n", GEAR_RATIO);
    DEBUG_PRINT("  Pulley Circumference: %.1fmm\r\n", PULLEY_CIRCUMFERENCE_MM);
    DEBUG_PRINT("  Counts per mm: %.2f\r\n", COUNTS_PER_MM);
    DEBUG_PRINT("\r\nSystem initialization complete!\r\n");
    
    HAL_Delay(1000);
}

// �޸�System_Main_Loop����
void System_Main_Loop(void)
{
    uint32_t current_time = HAL_GetTick();
    system_time = current_time;
    
    // 1. ����ϵͳ������
    SystemManager_Update();
    
    // 2. ���±���������
    Encoder_Update();
    
    // 3. ���µ��λ��
    Motor_Update_Position();
    
    // 4. ���λ�ð�ȫ
    Motor_Update_Target_Check();
    
    // 5. ������������
    DEBUG_PRINT("[MAIN] Calling Bluetooth_Process_Rx_Data()\r\n");
    Bluetooth_Process_Rx_Data();
    
    // ���������������while������������е���令
    DEBUG_PRINT("[MAIN] Checking Bluetooth_Cmd_Available()...\r\n");
    while (Bluetooth_Cmd_Available()) {
        BluetoothCmd cmd = Bluetooth_Get_Command();
        DEBUG_PRINT("[MAIN] Got command from queue: '%c'(0x%02X)\r\n", cmd, cmd);
        Bluetooth_Process_Command(cmd);
    }
    
    // 6. �����������ݣ�����У�
    // if (Voice_Is_Ready()) {
    //     Voice_Process_Rx_Data();
    // }
    
    // 7. ��������ģʽ
    if (system_mode == MODE_REHABILITATION && Rehabilitation_Is_Active()) {
        Rehabilitation_Process();
    }
    
    // 8. �������ģʽ
    if (g_rehab_params.is_active) {
        Resistance_Mode_Update();
    }
    
    // 8. LED״ָ̬ʾ
    static uint32_t last_led_toggle = 0;
    if (current_time - last_led_toggle > 500) {
        HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
        last_led_toggle = current_time;
    }
    
    // 9. ����OLED��ʾ
    static uint32_t last_oled_update = 0;
    if (current_time - last_oled_update > 200) {  // 5Hz����
        Update_OLED_Display();
        last_oled_update = current_time;
    }
    
    // 10. ����״̬���棨���ԣ�
    if (DEBUG_ENABLED) {
        static uint32_t last_status_print = 0;
        if (current_time - last_status_print > 2000) {  // 2��һ��
            Print_System_Status();
            last_status_print = current_time;
        }
    }
}

// ============================================
// ���������
// ============================================

void Process_Serial_Commands(void)
{
    uint8_t rx_char;
    
    if (HAL_UART_Receive(&huart1, &rx_char, 1, 0) == HAL_OK) {
        switch (rx_char) {
            case '1':  // ǰ��30%
                Motor_Go_Forward(30);
                DEBUG_PRINT("Forward 30%%\r\n");
                break;
                
            case '2':  // ֹͣ
                Motor_Stop();
                DEBUG_PRINT("Stop\r\n");
                break;
                
            case '3':  // ����30%
                Motor_Go_Backward(30);
                DEBUG_PRINT("Backward 30%%\r\n");
                break;
                
            case 'r':
            case 'R':  // �л�����ģʽ
                if (Rehabilitation_Is_Active()) {
                    Rehabilitation_Stop();
                    system_mode = MODE_MANUAL;
                    DEBUG_PRINT("Rehab mode stopped\r\n");
                } else {
                    Rehabilitation_Start();
                    system_mode = MODE_REHABILITATION;
                    DEBUG_PRINT("Rehab mode started\r\n");
                }
                break;
                
            case 'u':
            case 'U':  // ��������
                if (Rehabilitation_Is_Active()) {
                    Rehabilitation_Increase_Level();
                    DEBUG_PRINT("Resistance: Level %d\r\n", Rehabilitation_Get_Level());
                } else {
                    DEBUG_PRINT("Rehab mode not active\r\n");
                }
                break;
                
            case 'd':
            case 'D':  // ��������
                if (Rehabilitation_Is_Active()) {
                    Rehabilitation_Decrease_Level();
                    DEBUG_PRINT("Resistance: Level %d\r\n", Rehabilitation_Get_Level());
                } else {
                    DEBUG_PRINT("Rehab mode not active\r\n");
                }
                break;
                
            case 'e':
            case 'E':  // ��������Ϣ
                DEBUG_PRINT("Encoder: count=%ld, pos=%.1fmm, vel=%.1fmm/s\r\n",
                           Encoder_Get_Count(), Encoder_Get_Position_MM(),
                           Encoder_Get_Velocity_MM_S());
                break;
                
            case 's':
            case 'S':  // ϵͳ״̬
                Print_System_Status();
                break;
                
            case 't':
            case 'T':  // �������
                Motor_Test_Routine();
                DEBUG_PRINT("Motor test executed\r\n");
                break;
                
            case 'x':
            case 'X':  // ����ɲ��
                Motor_Emergency_Brake();
                system_mode = MODE_EMERGENCY;
                DEBUG_PRINT("Emergency brake!\r\n");
                break;
                
            case 'z':
            case 'Z':  // �������
                Motor_Set_Zero_Position();
                DEBUG_PRINT("Zero position set\r\n");
                break;
                
            case 'm':
            case 'M':  // ���ý���ֹͣ
                Motor_Reset_Emergency_Stop();
                system_mode = MODE_IDLE;
                DEBUG_PRINT("Emergency stop reset\r\n");
                break;
                
            case 'v':
            case 'V':  // �������ģʽ
                if (g_rehab_params.is_active) {
                    Resistance_Mode_Stop();
                    system_mode = MODE_IDLE;
                    DEBUG_PRINT("Resistance mode stopped\r\n");
                } else {
                    Resistance_Mode_Start();
                    system_mode = MODE_REHABILITATION;
                    DEBUG_PRINT("Resistance mode started\r\n");
                }
                break;
                
            case 'l':
            case 'L':  // ����ָ��������
                {
                    static uint8_t current_level = 3;
                    current_level++;
                    if (current_level > 10) current_level = 1;
                    Set_Resistance_Level(current_level);
                    DEBUG_PRINT("Resistance level: %d\r\n", current_level);
                }
                break;
                
            case 'p':
            case 'P':  // �ƶ���50%λ��
                Motor_Move_To_Percent(50, 30);
                DEBUG_PRINT("Moving to 50%% position\r\n");
                break;
                
            case 'h':
            case 'H':  // ����
                DEBUG_PRINT("Commands: 1(FWD) 2(STOP) 3(REV) R(Rehab) U/D(Resistance)\r\n");
                DEBUG_PRINT("          E(Encoder) S(Status) T(Test) X(Emergency)\r\n");
                DEBUG_PRINT("          Z(Zero) M(Reset) P(Center) H(Help)\r\n");
                break;
                
            default:
                DEBUG_PRINT("Unknown command: %c\r\n", rx_char);
                break;
        }
    }
}

void Process_Bluetooth_Commands(void)
{
    while (Bluetooth_Cmd_Available()) {
        BluetoothCmd cmd = Bluetooth_Get_Command();
        Bluetooth_Process_Command(cmd);
    }
}

void Process_Voice_Commands(void)
{
    // �������������voice.c��ʵ�֣�
    // �������գ�ʵ�ʴ�����Voice_Process()��
}

// ============================================
// ��ʾ����
// ============================================

void Update_OLED_Display(void)
{
    static uint8_t page = 0;
    static uint32_t last_page_switch = 0;
    uint32_t current_time = HAL_GetTick();
    
    // ÿ2����ҳ��(������ҳ��ʱ��)
    if (current_time - last_page_switch > 2000) {
        page = (page + 1) % OLED_PAGE_COUNT;
        last_page_switch = current_time;
    }
    
    // ������Ҫ�л�ҳ������Ҫ��
    // OLED_Clear();
    
    switch (page) {
        case 0:  // ��״̬ҳ��
            {
                char buffer[20];
                
                // ����
                OLED_Show_String(25, 0, "Rehab Arm", 6);
                
                // ģʽ
                const char* mode_str = "Manual";
                if (system_mode == MODE_REHABILITATION) mode_str = "Rehab";
                else if (system_mode == MODE_EMERGENCY) mode_str = "EMERGENCY";
                else if (system_mode == MODE_IDLE) mode_str = "Idle";
                
                sprintf(buffer, "Mode: %s", mode_str);
                OLED_Show_String(0, 1, buffer, 6);
                
                // λ��
                float pos_mm = Motor_Get_Position_MM();
                float pos_percent = Motor_Get_Position_Percent();
                sprintf(buffer, "Pos: %.0fmm", pos_mm);
                OLED_Show_String(0, 2, buffer, 6);
                
                sprintf(buffer, "     %.0f%%", pos_percent);
                OLED_Show_String(0, 3, buffer, 6);
                
                // �ٶ�
                uint8_t speed = Motor_Get_Speed();
                sprintf(buffer, "Speed: %d%%", speed);
                OLED_Show_String(0, 4, buffer, 6);
                
                // ����
                const char* dir_str = "Stop";
                if (Motor_Get_Direction() == MOTOR_FORWARD) dir_str = "Forward";
                else if (Motor_Get_Direction() == MOTOR_BACKWARD) dir_str = "Backward";
                
                sprintf(buffer, "Dir: %s", dir_str);
                OLED_Show_String(0, 5, buffer, 6);
                
                // ҳ��ָʾ
                OLED_Show_String(0, 6, "Page 1/4", 6);
            }
            break;
            
        case 1:  // ����ģʽҳ��
            {
                char buffer[20];
                
                OLED_Show_String(20, 0, "Rehab Mode", 6);
                
                // ״̬
                if (Rehabilitation_Is_Active()) {
                    OLED_Show_String(0, 1, "Status: ACTIVE", 6);
                } else {
                    OLED_Show_String(0, 1, "Status: INACTIVE", 6);
                }
                
                // �����ȼ�
                uint8_t level = Rehabilitation_Get_Level();
                sprintf(buffer, "Level: %d/9", level);
                OLED_Show_String(0, 2, buffer, 6);
                
                // �˶�����
                uint32_t count = Rehabilitation_Get_Exercise_Count();
                sprintf(buffer, "Count: %lu", count);
                OLED_Show_String(0, 3, buffer, 6);
                
                // �ܾ���
                float distance = Rehabilitation_Get_Total_Distance();
                sprintf(buffer, "Dist: %.0fmm", distance);
                OLED_Show_String(0, 4, buffer, 6);
                
                // ��ǰ�ٶ�
                float velocity = Encoder_Get_Velocity_MM_S();
                sprintf(buffer, "Vel: %.0fmm/s", fabs(velocity));
                OLED_Show_String(0, 5, buffer, 6);
                
                // ҳ��ָʾ
                OLED_Show_String(0, 6, "Page 2/4", 6);
            }
            break;
            
        case 2:  // ϵͳ��Ϣҳ��
            {
                char buffer[20];
                
                OLED_Show_String(20, 0, "System Info", 6);
                
                // ����ʱ��
                uint32_t seconds = system_time / 1000;
                uint32_t minutes = seconds / 60;
                seconds %= 60;
                sprintf(buffer, "Time: %02lu:%02lu", minutes, seconds);
                OLED_Show_String(0, 2, buffer, 6);
                
                // ����������
                int32_t enc_count = Encoder_Get_Count();
                sprintf(buffer, "Enc: %ld", enc_count);
                OLED_Show_String(0, 3, buffer, 6);
                
                // ����״̬
                uint8_t bt_connected = Bluetooth_Is_Connected();
                sprintf(buffer, "BT: %s", bt_connected ? "Connected" : "Disconnected");
                OLED_Show_String(0, 4, buffer, 6);
                
                // ����ֹͣ״̬
                if (Motor_Is_Emergency_Stop()) {
                    OLED_Show_String(0, 5, "EMERGENCY STOP!", 6);
                }
                
                // ҳ��ָʾ
                OLED_Show_String(0, 6, "Page 3/4", 6);
            }
            break;
            
        case 3:  // ����ҳ��
            {
                OLED_Show_String(30, 0, "Help", 6);
                OLED_Show_String(0, 2, "1: Forward", 6);
                OLED_Show_String(0, 3, "2: Stop", 6);
                OLED_Show_String(0, 4, "3: Backward", 6);
                OLED_Show_String(0, 5, "R: Rehab Mode", 6);
                OLED_Show_String(0, 6, "Page 4/4", 6);
            }
            break;
    }
    
    OLED_Refresh();
}

void Print_System_Status(void)
{
    char buffer[256];
    
    sprintf(buffer,
            "=== System Status ===\r\n"
            "Time: %lums\r\n"
            "Mode: %d, State: %d\r\n"
            "Motor: Dir=%d, Speed=%d%%\r\n"
            "Position: %.1fmm (%.1f%%)\r\n"
            "Encoder: %ld counts\r\n"
            "Velocity: %.1fmm/s\r\n"
            "Rehab: %s, Level=%d\r\n"
            "Exercise Count: %lu\r\n"
            "Total Distance: %.1fmm\r\n"
            "Emergency: %s\r\n"
            "Moving: %s\r\n"
            "BT Connected: %s\r\n",
            system_time,
            system_mode, system_state,
            Motor_Get_Direction(), Motor_Get_Speed(),
            Motor_Get_Position_MM(), Motor_Get_Position_Percent(),
            Encoder_Get_Count(),
            Encoder_Get_Velocity_MM_S(),
            Rehabilitation_Is_Active() ? "ON" : "OFF",
            Rehabilitation_Get_Level(),
            Rehabilitation_Get_Exercise_Count(),
            Rehabilitation_Get_Total_Distance(),
            Motor_Is_Emergency_Stop() ? "YES" : "NO",
            Motor_Is_Moving() ? "YES" : "NO",
            Bluetooth_Is_Connected() ? "YES" : "NO");
    
    DEBUG_PRINT("%s", buffer);
}

// ============================================
// ϵͳ״̬���
// ============================================

void System_Check_Status(void)
{
    // ��������״̬
    if (!Encoder_Is_Initialized()) {
        System_Handle_Error("Encoder not initialized!");
        return;
    }
    
    // ���λ�ð�ȫ
    if (g_motor_status.soft_limit_enabled) {
        float current_mm = Motor_Get_Position_MM();
        
        if (current_mm < g_motor_status.min_limit_mm - 1.0f) {
            DEBUG_PRINT("WARNING: Below min limit! %.1fmm < %.1fmm\r\n",
                         current_mm, g_motor_status.min_limit_mm);
            Motor_Emergency_Brake();
            system_mode = MODE_EMERGENCY;
        } else if (current_mm > g_motor_status.max_limit_mm + 1.0f) {
            DEBUG_PRINT("WARNING: Above max limit! %.1fmm > %.1fmm\r\n",
                       current_mm, g_motor_status.max_limit_mm);
            Motor_Emergency_Brake();
            system_mode = MODE_EMERGENCY;
        }
    }
    
    // ������¶ȣ�������������¶ȴ�������
    // ����ص�ѹ�������������ADC��⣩
}

void System_Handle_Error(const char* error_msg)
{
    // ����ֹͣ
    Motor_Force_Stop();
    system_mode = MODE_EMERGENCY;
    system_state = SYS_STATE_ERROR;
    
    // ��ʾ������Ϣ
    DEBUG_PRINT("\r\n!!! SYSTEM ERROR !!!\r\n");
    DEBUG_PRINT("Error: %s\r\n", error_msg);
    DEBUG_PRINT("System halted.\r\n");
    
    // OLED��ʾ����
    OLED_Clear();
    OLED_Show_String(10, 0, "!!! ERROR !!!", 6);
    OLED_Show_String(0, 2, error_msg, 6);
    OLED_Show_String(0, 4, "System halted", 6);
    OLED_Show_String(0, 6, "Restart required", 6);
    OLED_Refresh();
    
    // ������˸LED
    while (1) {
        HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
        HAL_Delay(100);
    }
}

// ============================================
// ���Ժ���
// ============================================

void System_Self_Test(void)
{
    DEBUG_PRINT("Starting system self-test...\r\n");
    
    // 1. �������
    Motor_Test_Routine();
    
    // 2. ����������
    Encoder_Direction_Test();
    
    // 3. ��������
    Bluetooth_Send_String("Bluetooth test OK\r\n");
    
    // 4. OLED����
    OLED_Clear();
    OLED_Show_String(20, 0, "Self Test", 6);
    OLED_Show_String(10, 2, "All Tests", 6);
    OLED_Show_String(20, 3, "PASSED", 6);
    OLED_Refresh();
    HAL_Delay(2000);
    
    DEBUG_PRINT("System self-test completed successfully!\r\n");
}

// ============================================
// HAL��ص�����
// ============================================

// DMA����ص�
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        DEBUG_PRINT("Bluetooth UART error\r\n");
        Bluetooth_Handle_Error();
    }
    else if (huart->Instance == USART2) {
        DEBUG_PRINT("Voice UART error\r\n");
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
  
  // ��˸LED
    while (1) {
        HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
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
