#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "main.h"
#include "config.h"
#include "spi.h"

#define DISPLAY_LANE1    0
#define DISPLAY_LANE2    1

#define ST7789_WIDTH     240
#define ST7789_HEIGHT    240

#define BLACK       0x0000
#define WHITE       0xFFFF
#define RED         0xF800
#define GREEN       0xE007
#define BLUE        0x001F
#define YELLOW      0xFFE0
#define CYAN        0x07FF
#define MAGENTA     0xF81F
#define ORANGE      0xFD20
#define GRAY        0x8410
#define DARKGREEN   0x03E0
#define DARKRED     0x8000

#define FONT_SIZE_SMALL   1
#define FONT_SIZE_NORMAL  2
#define FONT_SIZE_LARGE   3

void Display_Init(void);
void Display_InitScreen(uint8_t displayId);
void Display_SetBacklight(uint8_t displayId, uint8_t state);
void Display_Clear(uint8_t displayId, uint16_t color);
void Display_FillRect(uint8_t displayId, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void Display_DrawChar(uint8_t displayId, uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bgcolor, uint8_t size);
void Display_DrawString(uint8_t displayId, uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bgcolor, uint8_t size);
void Display_DrawVerticalString(uint8_t displayId, uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bgcolor, uint8_t size);
void Display_DrawNumber(uint8_t displayId, uint16_t x, uint16_t y, int32_t num, uint16_t color, uint16_t bgcolor, uint8_t size);
void Display_ShowSpeed(uint8_t displayId, uint8_t speed, AlertLevel_t level);
void Display_ShowWarning(uint8_t displayId, const char *msg, AlertLevel_t level);
void Display_ShowVehicleInfo(uint8_t displayId, VehicleType_t type, uint8_t speed);
void Display_ShowIdle(uint8_t displayId);
void Display_Update(uint8_t displayId);

#endif
