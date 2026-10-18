#ifndef _BEEP_H
#define _BEEP_H


#include "main.h"

#define BEEP(x)       x? HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);\
											  HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);

#endif
