#ifndef __SYSTEM_MANAGER_H
#define __SYSTEM_MANAGER_H

#include "config.h"
#include "app_globals.h"
// ============================================
// ????????
// ============================================
typedef enum {
    SYS_EVENT_NONE = 0,
    SYS_EVENT_STARTUP,
    SYS_EVENT_BT_CONNECTED,
    SYS_EVENT_BT_DISCONNECTED,
    SYS_EVENT_VOICE_CMD,
    SYS_EVENT_EMERGENCY_STOP,
    SYS_EVENT_POSITION_LIMIT,
    SYS_EVENT_ERROR,
    SYS_EVENT_WARNING
} SystemEvent;

// ============================================
// ????????
// ============================================
typedef struct {
    uint8_t initialized;           // ??????????
    uint8_t safe_mode;             // ????????
    uint32_t uptime_ms;            // ???????
    uint32_t last_event_time;      // ?????????
    SystemEvent last_event;        // ??????
    uint8_t event_count;           // ???????
    float power_voltage;           // ??????
    uint8_t temperature;           // ???
} SystemManager;

// ============================================
// ????????
// ============================================

// ?????????
void SystemManager_Init(void);
void SystemManager_Deinit(void);

// ??????
void SystemManager_Update(void);
void SystemManager_Handle_Event(SystemEvent event);
void SystemManager_Set_Safe_Mode(uint8_t enable);
uint8_t SystemManager_Is_Safe(void);

// ??????
uint32_t SystemManager_Get_Uptime(void);
char* SystemManager_Get_Uptime_String(void);

// ??????
void SystemManager_Log_Error(const char* error_msg);
void SystemManager_Clear_Errors(void);
uint8_t SystemManager_Get_Error_Count(void);

// ??????
uint8_t SystemManager_Check_Safety(void);
void SystemManager_Shutdown(void);


#endif /* __SYSTEM_MANAGER_H */
