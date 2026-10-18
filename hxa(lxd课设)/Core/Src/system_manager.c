#include "system_manager.h"
#include "config.h"
#include <stdio.h>

SystemStatus_t g_system = {
    .state = SYS_STATE_INIT,
    .mode = MODE_IDLE,
    .is_ready = 0,
    .system_time = 0,
    .debug_enabled = 1,
    .last_update = 0
};

void System_Manager_Init(void)
{
    g_system.state = SYS_STATE_INIT;
    g_system.mode = MODE_IDLE;
    g_system.is_ready = 0;
    g_system.system_time = 0;
    g_system.debug_enabled = 1;
    g_system.last_update = HAL_GetTick();
    
    // 初始化各个模块
    Motor_Controller_Init(NULL, 0, 0, 0);
    Bluetooth_Init(NULL);
    Voice_Init(NULL);
    // OLED_Init在main.c中通过OLED_Init_Safe()初始化
    // Encoder_Init在main.c中初始化
    Rehabilitation_Init();
    
    g_system.is_ready = 1;
    g_system.state = SYS_STATE_IDLE;
    
    DEBUG_PRINT("System Manager initialized\r\n");
}

void System_Manager_Run(void)
{
    if (!g_system.is_ready) return;
    
    g_system.system_time = HAL_GetTick();
    
    // 更新各个模块
    Encoder_Update();
    Motor_Update_Position();
    
    // 处理蓝牙数据
    Bluetooth_Process_Data();
    
    // 处理语音数据
    Voice_Process_Data();
    
    // 处理康复模式
    Rehabilitation_Process();
    
    // 更新OLED显示
    // OLED更新在main.c中处理
    
    g_system.last_update = HAL_GetTick();
}

void System_Change_Mode(OperationMode_t new_mode)
{
    g_system.mode = new_mode;
    
    switch(new_mode) {
        case MODE_IDLE:
            Motor_Stop();
            Rehabilitation_Stop();
            g_system.state = SYS_STATE_IDLE;
            break;
            
        case MODE_MANUAL:
            Motor_Stop();
            Rehabilitation_Stop();
            g_system.state = SYS_STATE_BLUETOOTH_CTRL;
            break;
            
        case MODE_REHABILITATION:
            Rehabilitation_Start();
            g_system.state = SYS_STATE_REHAB_MODE;
            break;
            
        case MODE_CALIBRATION:
            Motor_Stop();
            Rehabilitation_Stop();
            // 执行校准
            break;
            
        case MODE_EMERGENCY:
            Motor_Emergency_Stop();
            Rehabilitation_Stop();
            g_system.state = SYS_STATE_ERROR;
            break;
            
        default:
            g_system.mode = MODE_IDLE;
            g_system.state = SYS_STATE_ERROR;
            break;
    }
    
    DEBUG_PRINT("System mode changed to %d\r\n", new_mode);
}

void System_Emergency_Stop(void)
{
    Motor_Emergency_Stop();
    Rehabilitation_Stop();
    g_system.state = SYS_STATE_ERROR;
    
    DEBUG_PRINT("[EMERGENCY] System stopped\r\n");
}

uint8_t System_Is_Ready(void)
{
    return g_system.is_ready;
}

void System_Print_Status(void)
{
    DEBUG_PRINT("=== System Status ===\r\n");
    DEBUG_PRINT("State: %d\r\n", g_system.state);
    DEBUG_PRINT("Mode: %d\r\n", g_system.mode);
    DEBUG_PRINT("Ready: %d\r\n", g_system.is_ready);
    DEBUG_PRINT("System Time: %lu\r\n", g_system.system_time);
    DEBUG_PRINT("Debug: %d\r\n", g_system.debug_enabled);
    DEBUG_PRINT("Last Update: %lu\r\n", g_system.last_update);
    DEBUG_PRINT("=====================\r\n");
}

void System_Manager_Update(void)
{
    System_Manager_Run();
}

// 兼容旧接口 - 在main.c中实现

