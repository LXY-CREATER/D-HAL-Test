/* USER CODE BEGIN Header */

	/**
  ******************************************************************************
  * @file           : voice_handler.h
  * @brief          : Voice recognition handler header file
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __VOICE_HANDLER_H
#define __VOICE_HANDLER_H

#include "main.h"
#include "usart.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

// 语音缓冲区大小
#ifndef VOICE_RX_BUFFER_SIZE
#define VOICE_RX_BUFFER_SIZE    64
#endif
#ifndef VOICE_TX_BUFFER_SIZE
#define VOICE_TX_BUFFER_SIZE    64
#endif

// 语音命令定义
#define VOICE_CMD_FORWARD       "FWD"
#define VOICE_CMD_BACKWARD      "BWD"
#define VOICE_CMD_STOP          "STP"
#define VOICE_CMD_SPEED_UP      "SPU"
#define VOICE_CMD_SPEED_DOWN    "SPD"
#define VOICE_CMD_REHAB_ON      "RHO"
#define VOICE_CMD_REHAB_OFF     "RHF"
#define VOICE_CMD_RESIST_UP     "RSU"
#define VOICE_CMD_RESIST_DOWN   "RSD"
#define VOICE_CMD_EMERGENCY     "EMG"

/* USER CODE END Private defines */

/* USER CODE BEGIN Private typedefs */

// 语音状态
typedef struct {
    UART_HandleTypeDef* huart;
    uint8_t rx_buffer[VOICE_RX_BUFFER_SIZE];
    uint8_t tx_buffer[VOICE_TX_BUFFER_SIZE];
    uint8_t rx_index;
    uint8_t connected;
    uint32_t last_activity;
    char last_command[16];
    uint8_t new_command;
} VoiceState_t;

/* USER CODE END Private typedefs */

/* USER CODE BEGIN Private variables */

extern VoiceState_t g_voice_state;

/* USER CODE END Private variables */

/* USER CODE BEGIN Private function prototypes */

// 初始化语音模块
void Voice_Init(UART_HandleTypeDef* huart);

// 启动接收
void Voice_Start_Receive(void);

// 处理接收到的数据
void Voice_Process_Data(void);

// 处理语音命令
void Voice_Process_Command(void);

// 检查是否有新命令
uint8_t Voice_Has_Command(void);

// 获取最后命令
char* Voice_Get_Last_Command(void);

// 发送响应
void Voice_Send_Response(const char* response);

// 语音命令转动作
void Voice_Execute_Command(const char* cmd);

// UART接收完成回调
void Voice_UART_RxCpltCallback(UART_HandleTypeDef *huart);

/* USER CODE END Private function prototypes */

#endif /* __VOICE_HANDLER_H */
