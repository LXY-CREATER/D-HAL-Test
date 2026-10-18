/* USER CODE BEGIN Header */

	/**
  ******************************************************************************
  * @file           : system_manager.h
  * @brief          : System manager header file
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __SYSTEM_MANAGER_H
#define __SYSTEM_MANAGER_H

#include "main.h"
#include "motor_controller.h"
#include "bluetooth_handler.h"
#include "voice_handler.h"
#include "oled_display.h"
#include "encoder_reader.h"
#include "rehabilitation_mode.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* USER CODE BEGIN Private typedefs */

// 系统状态
typedef enum {
    SYS_STATE_INIT = 0,
    SYS_STATE_IDLE,           // 空闲状态
    SYS_STATE_BLUETOOTH_CTRL,
    SYS_STATE_VOICE_CTRL,
    SYS_STATE_REHAB_MODE,
    SYS_STATE_ERROR
} SystemState_t;

// 操作模式
typedef enum {
    MODE_IDLE = 0,
    MODE_MANUAL,
    MODE_REHABILITATION,
    MODE_CALIBRATION,
    MODE_EMERGENCY
} OperationMode_t;

// 系统状态结构 - 定义在system_manager.c中
typedef struct {
    SystemState_t state;
    OperationMode_t mode;
    uint8_t is_ready;
    uint32_t system_time;
    uint8_t debug_enabled;
    uint32_t last_update;
} SystemStatus_t;

/* USER CODE END Private typedefs */

/* USER CODE BEGIN Private variables */

// 系统状态
extern SystemStatus_t g_system;

/* USER CODE END Private variables */

/* USER CODE BEGIN Private function prototypes */

// 系统管理函数
void System_Manager_Init(void);
void System_Manager_Run(void);
void System_Manager_Update(void);
void System_Change_Mode(OperationMode_t new_mode);
void System_Emergency_Stop(void);

// 系统状态查询
uint8_t System_Is_Ready(void);
void System_Print_Status(void);

// 兼容旧接口
#define SystemManager_Init System_Manager_Init
#define SystemManager_Update System_Manager_Update

/* USER CODE END Private function prototypes */

#endif /* __SYSTEM_MANAGER_H */
