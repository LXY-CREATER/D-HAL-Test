#ifndef __UART_H
#define __UART_H

#include "stm32f1xx_hal.h"


#define UART_RX_BUFFER_SIZE 32  // ���建������С
#define UART_RX_TIMEOUT 100  // ���ʱʱ�䣨ms����100ms������������Ϊ�������

// UART����ṹ��
typedef struct {
    UART_HandleTypeDef* huart;       // UARTӲ�����
    uint8_t rx_buffer[32];           // ���ջ�����
    uint16_t rx_index;               // ��������
    uint8_t cmd_ready;               // ���������־
		uint32_t last_rx_time;           // ���һ�ν������ݵ�ʱ��������ڳ�ʱ�жϣ�
    void (*callback)(uint8_t*, uint16_t); // ������ص�
} UART_Handler;

// �ⲿ��������UART���
extern UART_Handler bluetooth_uart;

// ��������
void UART_Init(UART_Handler* huart_handler, UART_HandleTypeDef* huart, void (*callback)(uint8_t*, uint16_t));
void UART_SendString(UART_Handler* huart_handler, const char* str);
void UART_Process(UART_Handler* huart_handler);

#endif /* __UART_H */
