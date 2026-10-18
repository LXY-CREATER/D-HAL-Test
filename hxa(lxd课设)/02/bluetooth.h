#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include "config.h"
#include "app_globals.h"
// ============================================
// ????????
// ============================================
typedef struct {
    uint8_t rx_buffer[BT_RX_BUFFER_SIZE];      // ?????????
    uint8_t tx_buffer[BT_TX_BUFFER_SIZE];      // ?????????
    BluetoothCmd cmd_queue[BT_CMD_QUEUE_SIZE]; // ???????
    uint8_t rx_index;                          // ????????
    uint8_t cmd_head;                          // ????????
    uint8_t cmd_tail;                          // ???????¦Â
    uint8_t is_connected;                      // ??????
    uint32_t last_rx_time;                     // ?????????
    uint32_t error_count;                      // ???????
    uint8_t dma_active;                        // DMA??????
} BluetoothStatus;

// ============================================
// ????????
// ============================================

// ?????????
void Bluetooth_Init(UART_HandleTypeDef* uart);
void Bluetooth_Deinit(void);
void Bluetooth_Start_Receive(void);
void Bluetooth_Stop_Receive(void);

// ???????????
void Bluetooth_Process_Rx_Data(void);
void Bluetooth_Process_Command(BluetoothCmd cmd);
void Bluetooth_Parse_Data(uint8_t* data, uint16_t length);

// ??????§Û???
void Bluetooth_Put_Command(uint8_t cmd, uint8_t param);
uint8_t Bluetooth_Cmd_Available(void);
BluetoothCmd Bluetooth_Get_Command(void);
void Bluetooth_Clear_Queue(void);
void Bluetooth_Clear_Buffer(void);

// ???????
void Bluetooth_Send_Data(uint8_t* data, uint16_t length);
void Bluetooth_Send_String(const char* str);
void Bluetooth_Send_Response(const char* format, ...);
void Bluetooth_Send_Status(void);

// ??????
uint8_t Bluetooth_Is_Connected(void);
void Bluetooth_Set_Connected(uint8_t connected);
uint32_t Bluetooth_Get_Last_Rx_Time(void);

// ??????
void Bluetooth_Handle_Error(void);
void Bluetooth_Reset(void);

// ??????????
extern UART_HandleTypeDef* bluetooth_uart;

#endif /* __BLUETOOTH_H */
