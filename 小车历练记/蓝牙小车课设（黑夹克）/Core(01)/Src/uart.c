#include "uart.h"
#include "usart.h"
#include <string.h>
#include "stm32f1xx_hal.h"  // ����HAL_GetTick()

// ����UART���ʵ��������ʼ����ʱʱ�䣩
UART_Handler bluetooth_uart = {
    .huart = NULL,
    .rx_buffer = {0},
    .rx_index = 0,
    .cmd_ready = 0,
    .callback = NULL,
    .last_rx_time = 0  // ��ʼ��������ʱ��
};

// UART��ʼ��
void UART_Init(UART_Handler* huart_handler, UART_HandleTypeDef* huart, void (*callback)(uint8_t*, uint16_t))
{
    if(huart_handler && huart) {
        huart_handler->huart = huart;
        huart_handler->rx_index = 0;
        huart_handler->cmd_ready = 0;
        huart_handler->callback = callback;
        huart_handler->last_rx_time = 0;  // ���ó�ʱʱ��
        memset(huart_handler->rx_buffer, 0, UART_RX_BUFFER_SIZE);
        
        // �����жϽ��գ����յ�һ���ֽڣ�
        HAL_UART_Receive_IT(huart, &huart_handler->rx_buffer[huart_handler->rx_index], 1);
    }
}

// �����ַ���
void UART_SendString(UART_Handler* huart_handler, const char* str)
{
    if(huart_handler && huart_handler->huart && str) {
        HAL_UART_Transmit(huart_handler->huart, (uint8_t*)str, strlen(str), 100);
    }
}

// �������յ����������ʱ�жϣ�
void UART_Process(UART_Handler* huart_handler)
{
    if(huart_handler) {
        // ��ʱ�жϣ������趨ʱ��δ�յ������ݣ���Ϊ�������
        if(huart_handler->rx_index > 0 && 
           (HAL_GetTick() - huart_handler->last_rx_time > UART_RX_TIMEOUT)) {
            huart_handler->rx_buffer[huart_handler->rx_index] = '\0';
            huart_handler->cmd_ready = 1;
        }

        // ��������������
        if(huart_handler->cmd_ready && huart_handler->callback) {
            // ���ûص��������������ʱ������������������
            huart_handler->callback(huart_handler->rx_buffer, huart_handler->rx_index);
            
            // ���û����������������ж������ã��˴���������ݣ�
            memset(huart_handler->rx_buffer, 0, UART_RX_BUFFER_SIZE);
            huart_handler->cmd_ready = 0;
            huart_handler->last_rx_time = 0;

            // �Ƴ��˴���HAL_UART_Receive_IT���ã��������ж��е�������ͻ
        }
    }
}

// UART��������жϻص�
// UART��������жϻص�����
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // ����Ƿ�ΪĿ��UART������UART��
    if(huart->Instance == bluetooth_uart.huart->Instance) {
        // ����������ʱ�䣨���ڳ�ʱ�жϣ�
        bluetooth_uart.last_rx_time = HAL_GetTick();

        // ����������������з����س�����ﵽ4���ַ�
        if(bluetooth_uart.rx_buffer[bluetooth_uart.rx_index] == '\n' || 
           bluetooth_uart.rx_buffer[bluetooth_uart.rx_index] == '\r' || 
           bluetooth_uart.rx_index >= 4) {  
            bluetooth_uart.rx_buffer[bluetooth_uart.rx_index] = '\0'; // �ַ�����β
            bluetooth_uart.cmd_ready = 1; // ����������

            // �ؼ��޸�1����������������ȷ���´ν��մӻ�������ʼλ�ÿ�ʼ
            uint16_t current_len = bluetooth_uart.rx_index; // �ݴ浱ǰ����
            bluetooth_uart.rx_index = 0;

            // �ؼ��޸�2���������������жϣ��������ݶ�ʧ
            HAL_UART_Receive_IT(bluetooth_uart.huart, &bluetooth_uart.rx_buffer[bluetooth_uart.rx_index], 1);
        } else {
            bluetooth_uart.rx_index++;
            // �������������
            if(bluetooth_uart.rx_index >= UART_RX_BUFFER_SIZE - 1) {
                bluetooth_uart.rx_index = 0;
                memset(bluetooth_uart.rx_buffer, 0, UART_RX_BUFFER_SIZE);
                UART_SendString(&bluetooth_uart, "Command too long!\n");
                // �������������
                HAL_UART_Receive_IT(bluetooth_uart.huart, &bluetooth_uart.rx_buffer[bluetooth_uart.rx_index], 1);
            } else {
                // ����������һ���ַ�
                HAL_UART_Receive_IT(bluetooth_uart.huart, &bluetooth_uart.rx_buffer[bluetooth_uart.rx_index], 1);
            }
        }
    }
}
