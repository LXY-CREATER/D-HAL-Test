#include "uart.h"
#include "usart.h"
#include "motor.h"
uint8_t bluetooth_rx_data;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	 if (huart->Instance == USART1) 
		 {
        Bluetooth_ProcessCommand(bluetooth_rx_data);    
        HAL_UART_Receive_IT(&huart1, &bluetooth_rx_data, 1); 
    }
}
int  Bluetooth_ProcessCommand(uint8_t cmd) {
    switch(cmd) {
        case 'F':  
            Move_Front(255);
            break;
        case 'B':  
            Move_Back(255);
            break;
        case 'L':  
            Move_Left(255);
            break;
        case 'R':  
            Move_Right(255);
            break;
        case 'S':  
            Move_Stop();
            break;
        case 'Q':  
            Move_LeftFront(255);
            break;
        case 'E':  
            Move_RightFront(255);
            break;
        default:
            break;
    }
		
}
