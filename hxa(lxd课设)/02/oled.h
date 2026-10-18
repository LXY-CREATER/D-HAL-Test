#ifndef __OLED_H
#define __OLED_H

#include "config.h"
#include "common.h"
// ============================================
// OLED命令/数据定义
// ============================================
#define OLED_CMD     0
#define OLED_DATA    1

// ============================================
// 函数声明
// ============================================

// 基础函数
void OLED_Init(void);
void OLED_Init_Safe(void);
void OLED_Clear(void);
void OLED_Clear_Safe(void);
void OLED_Clear_Display(void);
void OLED_Refresh(void);
void OLED_Refresh_Safe(void);
void OLED_Refresh_Partial(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void OLED_Set_Pos(uint8_t x, uint8_t y);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Fill(uint8_t fill_Data);

// 显示函数
void OLED_Show_Char(uint8_t x, uint8_t y, uint8_t chr, uint8_t size);
void OLED_Show_String(uint8_t x, uint8_t y, const char *str, uint8_t size);
void OLED_Show_String_Safe(uint8_t x, uint8_t y, const char *str, uint8_t size);
void OLED_Show_Num(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);
void OLED_Show_SignedNum(uint8_t x, uint8_t y, int32_t num, uint8_t len, uint8_t size);
void OLED_Show_HexNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);
void OLED_Show_Chinese(uint8_t x, uint8_t y, uint8_t no, uint8_t size);

// 绘图函数
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t);
void OLED_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t mode);

// 显示模式
void Show_Default_Mode(void);
void Show_Bluetooth_Mode(void);
void Show_Voice_Mode(void);
void Show_Rehabilitation_Mode(void);
void OLED_Show_Main_Status(void);
void OLED_Show_Motor_Info(void);
void OLED_Show_Rehab_Mode(void);
void OLED_Set_Page(uint8_t page);

// 系统函数
void OLED_Update_Display(void);
void OLED_Show_Error(const char* error_msg);
void OLED_Set_Brightness(uint8_t brightness);
void OLED_Force_Refresh(void);

// 测试函数
void OLED_Simple_Test(void);

#endif /* __OLED_H */
