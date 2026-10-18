#ifndef __KEY_H
#define __KEY_H

#include "main.h"

// 按键扫描函数：返回当前按下的键（无按键返回KEY_NONE）
KeyType Key_Scan(void);
// 读取电位器角度（0-180）
uint16_t Potentiometer_GetAngle(void);
void KEYPAD_DebugOutput(KeyType key);
#endif
