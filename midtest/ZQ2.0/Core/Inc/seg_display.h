#ifndef __SEG_DISPLAY_H
#define __SEG_DISPLAY_H

#include "main.h"

// 初始化数码管（需在main中调用）
void SegDisplay_Init(void);
// 更新显示的角度值
void SegDisplay_Update(uint16_t angle);
// 闪烁n次（确认后调用）
void SegDisplay_Blink(uint8_t times);

#endif
