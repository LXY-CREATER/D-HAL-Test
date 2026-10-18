#ifdef UART_H
#define UART_H

void HAL_UART_RxCallback(UART_HandleTypeDef*huart);
void Bluetooth_ProcessCommand(uint8_t cmd);

#endif
