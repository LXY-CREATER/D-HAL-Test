// oled_display.h
#ifndef __OLED_DISPLAY_H
#define __OLED_DISPLAY_H

#include "main.h"
#include "i2c.h"
#include "oled.h"  // 包含新的OLED驱动

// 显示页面枚举
typedef enum {
    PAGE_MAIN_STATUS = 0,
    PAGE_MOTOR_INFO,
    PAGE_REHAB_MODE,
    PAGE_ERROR
} DisplayPage_t;

// 显示模式枚举
typedef enum {
    DISPLAY_NORMAL = 0,
    DISPLAY_DIAGNOSTIC,
    DISPLAY_MINIMAL
} DisplayMode_t;

// OLED状态
typedef struct {
    DisplayPage_t current_page;
    DisplayMode_t display_mode;
    uint32_t last_update;
    uint8_t need_refresh;
    uint8_t is_initialized;
    uint8_t brightness;
} OLEDStatus_t;

// 全局OLED状态
extern OLEDStatus_t g_oled;

// 初始化函数
void OLED_Display_Init(I2C_HandleTypeDef* i2c);
void OLED_Deinit(void);

// 页面控制
void OLED_Set_Page(DisplayPage_t page);
void OLED_Set_Mode(DisplayMode_t mode);
void OLED_Force_Refresh(void);

// 显示函数
void OLED_Update_Display(void);
void OLED_Show_Main_Status(void);
void OLED_Show_Motor_Info(void);
void OLED_Show_Rehab_Mode(void);
void OLED_Show_Error(const char* error_msg);

// 工具函数
void OLED_Clear(void);
void OLED_Set_Brightness(uint8_t brightness);

#endif /* __OLED_DISPLAY_H */
