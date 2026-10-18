#include "system_manager.h"
#include "all_includes.h"
#include <string.h>

// ============================================
// 全局变量定义
// ============================================

SystemManager sys_mgr = {
    .initialized = 0,
    .safe_mode = 1,
    .uptime_ms = 0,
    .last_event_time = 0,
    .last_event = SYS_EVENT_NONE,
    .event_count = 0,
    .power_voltage = 12.0f,
    .temperature = 25
};

// 错误日志缓冲区
static char error_log[10][64];
static uint8_t error_index = 0;

// ============================================
// 初始化函数
// ============================================

void SystemManager_Init(void)
{
    // 清空错误日志
    memset(error_log, 0, sizeof(error_log));
    error_index = 0;
    
    // 初始化时间
    sys_mgr.uptime_ms = 0;
    sys_mgr.last_event_time = HAL_GetTick();
    
    // 设置为安全模式
    sys_mgr.safe_mode = 1;
    sys_mgr.initialized = 1;
    
    DEBUG_PRINT("System Manager initialized\r\n");
}

void SystemManager_Deinit(void)
{
    sys_mgr.initialized = 0;
    SystemManager_Shutdown();
}

// ============================================
// 状态管理
// ============================================

void SystemManager_Update(void)
{
    static uint32_t last_update = 0;
    uint32_t current_time = HAL_GetTick();
    
    // 每秒更新一次
    if (current_time - last_update >= 1000) {
        sys_mgr.uptime_ms += 1000;
        last_update = current_time;
        
        // 定期安全检查
        if (!SystemManager_Check_Safety()) {
            SystemManager_Handle_Event(SYS_EVENT_WARNING);
        }
    }
}

void SystemManager_Handle_Event(SystemEvent event)
{
    if (!sys_mgr.initialized) return;
    
    sys_mgr.last_event = event;
    sys_mgr.last_event_time = HAL_GetTick();
    sys_mgr.event_count++;
    
    switch (event) {
        case SYS_EVENT_EMERGENCY_STOP:
            DEBUG_PRINT("!!! EMERGENCY STOP EVENT !!!\r\n");
            Motor_Emergency_Brake();
            sys_mgr.safe_mode = 1;
            break;
            
        case SYS_EVENT_POSITION_LIMIT:
            DEBUG_PRINT("Position limit reached\r\n");
            Motor_Smooth_Stop();
            break;
            
        case SYS_EVENT_ERROR:
            DEBUG_PRINT("System error event\r\n");
            sys_mgr.safe_mode = 1;
            break;
            
        case SYS_EVENT_BT_CONNECTED:
            DEBUG_PRINT("Bluetooth connected\r\n");
            break;
            
        case SYS_EVENT_BT_DISCONNECTED:
            DEBUG_PRINT("Bluetooth disconnected\r\n");
            break;
            
        default:
            break;
    }
}

void SystemManager_Set_Safe_Mode(uint8_t enable)
{
    sys_mgr.safe_mode = enable;
    
    if (enable) {
        Motor_Smooth_Stop();
        DEBUG_PRINT("Safe mode enabled\r\n");
    } else {
        DEBUG_PRINT("Safe mode disabled\r\n");
    }
}

uint8_t SystemManager_Is_Safe(void)
{
    return sys_mgr.safe_mode;
}

// ============================================
// 时间管理
// ============================================

uint32_t SystemManager_Get_Uptime(void)
{
    return sys_mgr.uptime_ms;
}

char* SystemManager_Get_Uptime_String(void)
{
    static char time_str[16];
    uint32_t seconds = sys_mgr.uptime_ms / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    
    seconds %= 60;
    minutes %= 60;
    
    sprintf(time_str, "%02lu:%02lu:%02lu", hours, minutes, seconds);
    return time_str;
}

// ============================================
// 错误处理
// ============================================

void SystemManager_Log_Error(const char* error_msg)
{
    if (!error_msg) return;
    
    // 记录错误
    strncpy(error_log[error_index], error_msg, 63);
    error_log[error_index][63] = '\0';
    
    error_index = (error_index + 1) % 10;
    
    // 触发错误事件
    SystemManager_Handle_Event(SYS_EVENT_ERROR);
    
    DEBUG_PRINT("Error logged: %s\r\n", error_msg);
}

void SystemManager_Clear_Errors(void)
{
    memset(error_log, 0, sizeof(error_log));
    error_index = 0;
    DEBUG_PRINT("Error log cleared\r\n");
}

uint8_t SystemManager_Get_Error_Count(void)
{
    return error_index;
}

// ============================================
// 安全检查
// ============================================

uint8_t SystemManager_Check_Safety(void)
{
    // 1. 检查位置安全
    float position = Encoder_Get_Position_MM();
    if (position < 0 || position > PHYSICAL_MAX_MM) {
        SystemManager_Log_Error("Position out of range");
        return 0;
    }
    
    // 2. 检查电机温度（模拟）
    if (sys_mgr.temperature > 60) {
        SystemManager_Log_Error("Motor temperature too high");
        return 0;
    }
    
    // 3. 检查电源电压
    if (sys_mgr.power_voltage < 10.0f || sys_mgr.power_voltage > 14.0f) {
        SystemManager_Log_Error("Power voltage abnormal");
        return 0;
    }
    
    return 1;
}

void SystemManager_Shutdown(void)
{
    DEBUG_PRINT("System shutdown initiated\r\n");
    
    // 1. 停止电机
    Motor_Force_Stop();
    
    // 2. 清空所有缓冲区
    Bluetooth_Clear_Buffer();
    // Voice_Clear_Buffer(); // 如果有语音模块
    
    // 3. 显示关机信息
    OLED_Clear();
    OLED_Show_String(10, 0, "SYSTEM", 6);
    OLED_Show_String(10, 2, "SHUTDOWN", 6);
    OLED_Refresh();
    
    // 4. 关闭外设
    HAL_Delay(1000);
    
    DEBUG_PRINT("System shutdown complete\r\n");
}
