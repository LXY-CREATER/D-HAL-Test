/* USER CODE BEGIN Header */

	/**
  ******************************************************************************
  * @file           : oled_display.h
  * @brief          : OLED display handler header file
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __OLED_DISPLAY_H
#define __OLED_DISPLAY_H

#include "main.h"
#include "i2c.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

// OLED参数
#define OLED_I2C_ADDRESS         0x78
#define OLED_WIDTH               128
#define OLED_HEIGHT              64
#define OLED_PAGE_COUNT          8
#define OLED_BUFFER_SIZE         (OLED_WIDTH * OLED_HEIGHT / 8)

// OLED命令
#define OLED_CMD_DISPLAY_OFF     0xAE
#define OLED_CMD_DISPLAY_ON      0xAF
#define OLED_CMD_SET_CONTRAST    0x81
#define OLED_CMD_NORMAL_DISPLAY  0xA6
#define OLED_CMD_INVERT_DISPLAY  0xA7
#define OLED_CMD_DISPLAY_RAM     0xA4
#define OLED_CMD_DISPLAY_ALL_ON  0xA5
#define OLED_CMD_SET_DISPLAY_OFFSET 0xD3
#define OLED_CMD_SET_COMPINS     0xDA
#define OLED_CMD_SET_VCOM_DETECT 0xDB
#define OLED_CMD_SET_DISPLAY_CLOCK_DIV 0xD5
#define OLED_CMD_SET_PRECHARGE   0xD9
#define OLED_CMD_SET_MULTIPLEX   0xA8
#define OLED_CMD_SET_LOW_COLUMN  0x00
#define OLED_CMD_SET_HIGH_COLUMN 0x10
#define OLED_CMD_SET_START_LINE  0x40
#define OLED_CMD_MEMORY_MODE     0x20
#define OLED_CMD_COLUMN_ADDR     0x21
#define OLED_CMD_PAGE_ADDR       0x22
#define OLED_CMD_COM_SCAN_INC    0xC0
#define OLED_CMD_COM_SCAN_DEC    0xC8
#define OLED_CMD_SEG_REMAP       0xA0
#define OLED_CMD_CHARGE_PUMP     0x8D
#define OLED_CMD_SET_PAGE_START  0xB0

/* USER CODE END Private defines */

/* USER CODE BEGIN Private typedefs */

typedef struct {
    I2C_HandleTypeDef* hi2c;
    uint8_t address;
    uint8_t buffer[OLED_BUFFER_SIZE];
    uint8_t initialized;
    uint8_t inverted;
} OLED_State_t;

/* USER CODE END Private typedefs */

/* USER CODE BEGIN Private variables */

extern OLED_State_t g_oled_state;

/* USER CODE END Private variables */

/* USER CODE BEGIN Private function prototypes */

// 初始化OLED
void OLED_Init_Safe(void);
void OLED_Clear_Safe(void);
void OLED_Refresh_Safe(void);
void OLED_Show_String(uint8_t x, uint8_t y, const char* str, uint8_t size);

// OLED基础函数
void OLED_Init(I2C_HandleTypeDef* hi2c, uint8_t address);

// 清屏
void OLED_Clear(void);

// 刷新显示
void OLED_Refresh(void);

// 关闭显示
void OLED_Display_Off(void);

// 开启显示
void OLED_Display_On(void);

// 设置对比度
void OLED_Set_Contrast(uint8_t contrast);

// 反转显示
void OLED_Invert(uint8_t invert);

// 画点
void OLED_Draw_Pixel(uint8_t x, uint8_t y, uint8_t color);

// 画直线
void OLED_Draw_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);

// 画矩形
void OLED_Draw_Rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);

// 填充矩形
void OLED_Fill_Rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);

// 显示字符
void OLED_Draw_Char(uint8_t x, uint8_t y, char ch, const uint8_t* font, uint8_t font_width, uint8_t font_height, uint8_t color);

// 显示字符串
void OLED_Draw_String(uint8_t x, uint8_t y, const char* str, const uint8_t* font, uint8_t font_width, uint8_t font_height, uint8_t color);

// 显示数字
void OLED_Draw_Number(uint8_t x, uint8_t y, int32_t num, const uint8_t* font, uint8_t font_width, uint8_t font_height, uint8_t color);

// 显示浮点数
void OLED_Draw_Float(uint8_t x, uint8_t y, float num, uint8_t decimals, const uint8_t* font, uint8_t font_width, uint8_t font_height, uint8_t color);

// 显示进度条
void OLED_Draw_Progress_Bar(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t progress, uint8_t color);

// 显示系统状态
void OLED_Display_System_Status(void);

// 显示康复模式状态
void OLED_Display_Rehab_Status(void);

/* USER CODE END Private function prototypes */

#endif /* __OLED_DISPLAY_H */
