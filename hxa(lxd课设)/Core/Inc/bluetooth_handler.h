/* USER CODE BEGIN Header */

	/**
  ******************************************************************************
  * @file           : bluetooth_handler.h
  * @brief          : Bluetooth communication handler header file
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __BLUETOOTH_HANDLER_H
#define __BLUETOOTH_HANDLER_H

#include "main.h"
#include "usart.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

// 蓝牙缓冲区大小
#ifndef BT_RX_BUFFER_SIZE
#define BT_RX_BUFFER_SIZE    128
#endif
#ifndef BT_TX_BUFFER_SIZE
#define BT_TX_BUFFER_SIZE    128
#endif
#ifndef BT_CMD_QUEUE_SIZE
#define BT_CMD_QUEUE_SIZE    20
#endif

// 蓝牙命令定义
#define BT_CMD_STOP          '0'
#define BT_CMD_FORWARD       '1'
#define BT_CMD_BACKWARD      '2'
#define BT_CMD_BRAKE         '3'
#define BT_CMD_FORWARD_30    '4'
#define BT_CMD_BACKWARD_30   '5'
#define BT_CMD_FORWARD_70    '6'
#define BT_CMD_BACKWARD_70   '7'
#define BT_CMD_MOVE_CENTER   'M'
#define BT_CMD_SET_ZERO      'Z'
#define BT_CMD_REHAB_MODE    'R'
#define BT_CMD_REHAB_PLUS    'U'
#define BT_CMD_REHAB_MINUS   'D'
#define BT_CMD_EMERGENCY     'X'

// 蓝牙命令执行状态
extern volatile uint8_t g_bt_command_executing;
#define BT_CMD_STATUS        'S'
#define BT_CMD_POSITION      'P'
#define BT_CMD_ENCODER       'E'
#define BT_CMD_IMPEDANCE     'I'
#define BT_CMD_CALIBRATE     'C'
#define BT_CMD_MOTOR_TEST    'T'
#define BT_CMD_VERSION       'V'
#define BT_CMD_HELP          'H'

/* USER CODE END Private defines */

/* USER CODE BEGIN Private typedefs */

// 蓝牙命令结构
typedef struct {
    uint8_t command;
    uint8_t parameter;
    uint32_t timestamp;
} BluetoothCmd_t;

// 蓝牙状态
typedef struct {
    UART_HandleTypeDef* huart;
    uint8_t rx_buffer[BT_RX_BUFFER_SIZE];
    uint8_t tx_buffer[BT_TX_BUFFER_SIZE];
    uint8_t rx_index;
    uint8_t tx_index;
    uint8_t connected;
    uint32_t last_activity;
    uint8_t cmd_queue[BT_CMD_QUEUE_SIZE];
    uint8_t cmd_queue_head;
    uint8_t cmd_queue_tail;
} BluetoothState_t;

/* USER CODE END Private typedefs */

/* USER CODE BEGIN Private variables */

extern BluetoothState_t g_bluetooth_state;

/* USER CODE END Private variables */

/* USER CODE BEGIN Private function prototypes */

// 初始化蓝牙
void Bluetooth_Init(UART_HandleTypeDef* huart);

// 启动接收
void Bluetooth_Start_Receive(void);

// 发送数据
void Bluetooth_Send(const uint8_t* data, uint16_t len);

// 发送字符串
void Bluetooth_Send_String(const char* str);

// 发送格式化字符串
void Bluetooth_Send_Format(const char* format, ...);

// 处理接收到的数据
void Bluetooth_Process_Data(void);

// 处理命令
void Bluetooth_Process_Command(uint8_t cmd);

// 检查是否有新命令
uint8_t Bluetooth_Has_Command(void);

// 获取命令
uint8_t Bluetooth_Get_Command(void);

// 检查是否有可用命令
uint8_t Bluetooth_Cmd_Available(void);

// 处理接收数据
void Bluetooth_Process_Rx_Data(void);

// 蓝牙命令类型
typedef uint8_t BluetoothCmd;

// 发送状态信息
void Bluetooth_Send_Status(void);

// 发送位置信息
void Bluetooth_Send_Position(void);

// 检查蓝牙是否连接
uint8_t Bluetooth_Is_Connected(void);

// 处理蓝牙错误
void Bluetooth_Handle_Error(void);

// UART接收完成回调
void Bluetooth_UART_RxCpltCallback(UART_HandleTypeDef *huart);

/* USER CODE END Private function prototypes */

#endif /* __BLUETOOTH_HANDLER_H */
