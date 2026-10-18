#ifndef __USART_CONFIG_H
#define __USART_CONFIG_H

#include "main.h"
#include "config.h"
#include "usart.h"

#define RX_BUFFER_SIZE    128

void UsartConfig_Init(void);
void UsartConfig_Process(void);
void UsartConfig_SendString(const char *str);
void UsartConfig_SendNumber(int32_t num);
void UsartConfig_PrintConfig(void);

#endif
