#include "rehabilitation_arm.h"
#include "oled.h"
#include "motor_type.h"
#include "config.h"  // 使用你的config.h
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern I2C_HandleTypeDef hi2c1;

// 全局变量定义
uint8_t bluetooth_rx_buffer[UART_BUFFER_SIZE];
uint8_t voice_rx_buffer[UART_BUFFER_SIZE];
uint8_t bluetooth_tx_buffer[UART_BUFFER_SIZE];
uint8_t voice_tx_buffer[UART_BUFFER_SIZE];

// 内部变量
static uint32_t system_tick = 0;
static uint8_t system_initialized = 0;
static uint8_t oled_update_enabled = 1;

// 调试宏
#define DEBUG_PRINT(fmt, ...) \
    do { \
        char debug_msg[128]; \
        sprintf(debug_msg, fmt, ##__VA_ARGS__); \
        HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 100); \
    } while(0)

// 系统初始化
void System_Init_Rehabilitation(void)
{
    if(system_initialized) return;
    
    DEBUG_PRINT("[REHAB] Starting rehabilitation system initialization\n");
    
    // 初始化OLED
    DEBUG_PRINT("[REHAB] Initializing OLED...\n");
    OLED_Init_Safe();
    OLED_Clear_Safe();
    OLED_Show_String_Safe(0, 0, "Rehab Arm", 6);
    OLED_Show_String_Safe(0, 2, "Initializing...", 6);
    OLED_Refresh_Safe();
    
    // 初始化LED
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
    
    system_initialized = 1;
    system_tick = HAL_GetTick();
    
    OLED_Show_String_Safe(0, 4, "Ready", 6);
    OLED_Refresh_Safe();
    HAL_Delay(500);
    
    DEBUG_PRINT("[REHAB] Rehabilitation system initialized\n");
}

// 安全版本显示更新（显示当前模式）
void Display_Update_Safe(void)
{
    // 检查I2C总线状态
    if(HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
    {
        return;  // I2C不可用，跳过显示更新
    }
    
    static uint8_t display_page = 0;
    static uint32_t last_page_change = 0;
    uint32_t current_time = HAL_GetTick();
    
    // 每5秒切换显示页面
    if(current_time - last_page_change > 5000)
    {
        display_page = (display_page + 1) % 3;
        last_page_change = current_time;
    }
    
    OLED_Clear_Safe();
    
    // 第一行始终显示当前模式
    char mode_str[20];
    switch(system_status.mode)
    {
        case MODE_BLUETOOTH:      sprintf(mode_str, "BT Mode"); break;
        case MODE_VOICE:          sprintf(mode_str, "Voice Mode"); break;
        case MODE_REHABILITATION: sprintf(mode_str, "Rehab Mode"); break;
        case MODE_TEACH_PLAYBACK: sprintf(mode_str, "Teach Mode"); break;
        case MODE_SETTINGS:       sprintf(mode_str, "Settings"); break;
        case MODE_CALIBRATION:    sprintf(mode_str, "Calibration"); break;
        default:                  sprintf(mode_str, "Unknown Mode"); break;
    }
    OLED_Show_String_Safe(0, 0, mode_str, 6);
    
    if(display_page == 0)
    {
        // 页面1：基本状态
        char line2[20], line3[20], line4[20];
        sprintf(line2, "Speed: %3d%%", system_status.motor_speed);
        sprintf(line3, "Position: %d", system_status.encoder_position);
        sprintf(line4, "B: %3d%% T: %2dC", 
                system_status.battery_level, 
                system_status.temperature);
        
        OLED_Show_String_Safe(0, 2, line2, 6);
        OLED_Show_String_Safe(0, 4, line3, 6);
        OLED_Show_String_Safe(0, 6, line4, 6);
    }
    else if(display_page == 1)
    {
        // 页面2：阻力模式信息
        if(system_status.mode == MODE_REHABILITATION)
        {
            char line2[20];
            sprintf(line2, "Resist: Level %d", system_status.resistance_level);
            
            OLED_Show_String_Safe(0, 2, line2, 6);
            OLED_Show_String_Safe(0, 4, "Push forward", 6);
            OLED_Show_String_Safe(0, 6, "for resistance", 6);
        }
        else
        {
            OLED_Show_String_Safe(0, 2, "F:Forward", 6);
            OLED_Show_String_Safe(0, 4, "B:Backward", 6);
            OLED_Show_String_Safe(0, 6, "S:Stop R:Resist", 6);
        }
    }
    else
    {
        // 页面3：帮助信息
        OLED_Show_String_Safe(0, 2, "+/-:Speed", 6);
        OLED_Show_String_Safe(0, 4, "U/D:Resist Level", 6);
        OLED_Show_String_Safe(0, 6, "T:Test E:Stop", 6);
    }
    
    OLED_Refresh_Safe();
}

// 主任务
void Main_Task_Rehabilitation(void)
{
    static uint32_t last_display_update = 0;
    static uint32_t last_status_send = 0;
    uint32_t current_time = HAL_GetTick();
    
    // 系统计时
    system_tick = current_time;
    
    // 显示更新（每秒一次）
    if(oled_update_enabled && current_time - last_display_update >= 1000)
    {
        Display_Update_Safe();
        last_display_update = current_time;
    }
    
    // 发送状态（每5秒一次）
    if(current_time - last_status_send >= 5000)
    {
        Send_System_Status();
        last_status_send = current_time;
    }
}

static DisplayManager_t display_manager = {0};

void OLED_Update_Task(void)
{
    static uint32_t last_update = 0;
    uint32_t current_time = HAL_GetTick();
    
    // 非阻塞检查：仅当I2C空闲且时间到才更新
    if(current_time - last_update < 100) // 10Hz刷新率
        return;
    
    if(HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
        return;
    
    last_update = current_time;
    
    // 根据模式选择显示内容
    switch(system_status.mode)
    {
        case MODE_BLUETOOTH:
            Show_Bluetooth_Mode();
            break;
        case MODE_VOICE:
            Show_Voice_Mode();
            break;
        case MODE_REHABILITATION:
            Show_Rehabilitation_Mode();
            break;
        default:
            Show_Default_Mode();
            break;
    }
}

//// 快速显示函数（避免字符串操作）
//void Show_Rehabilitation_Mode(void)
//{
//    // 清屏（仅清除需要更新的区域）
//    OLED_Clear_Rect(0, 0, 128, 64);
//    
//    // 第一行：模式
//    OLED_Show_String_Safe(0, 0, "康复模式", 6);
//    
//    // 第二行：阻力等级
//    char line2[16];
//    sprintf(line2, "阻力等级:%d", system_status.resistance_level);
//    OLED_Show_String_Safe(0, 2, line2, 6);
//    
//    // 第三行：当前位置
//    char line3[20];
//    sprintf(line3, "位置:%6ld", system_status.encoder_position);
//    OLED_Show_String_Safe(0, 4, line3, 6);
//    
//    // 第四行：速度
//    char line4[20];
//    sprintf(line4, "速度:%4d", system_status.encoder_speed);
//    OLED_Show_String_Safe(0, 6, line4, 6);
//    
//    // 立即刷新（但使用非阻塞方式）
//    OLED_Refresh_Partial(0, 0, 128, 64);
//}
