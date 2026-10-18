#include "display.h"
#include "usart_config.h"
#include "FONT.h"
#include "spi.h"
#include "config.h"
#include "decision_logic.h"
#include <stdio.h>

static const uint8_t font5x8[96][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x5F, 0x00, 0x00},
    {0x00, 0x07, 0x00, 0x07, 0x00},
    {0x14, 0x7F, 0x14, 0x7F, 0x14},
    {0x24, 0x2A, 0x7F, 0x2A, 0x12},
    {0x23, 0x13, 0x08, 0x64, 0x62},
    {0x36, 0x49, 0x55, 0x22, 0x50},
    {0x00, 0x05, 0x03, 0x00, 0x00},
    {0x00, 0x1C, 0x22, 0x41, 0x00},
    {0x00, 0x41, 0x22, 0x1C, 0x00},
    {0x14, 0x08, 0x3E, 0x08, 0x14},
    {0x08, 0x08, 0x3E, 0x08, 0x08},
    {0x00, 0x50, 0x30, 0x00, 0x00},
    {0x08, 0x08, 0x08, 0x08, 0x08},
    {0x00, 0x60, 0x60, 0x00, 0x00},
    {0x20, 0x10, 0x08, 0x04, 0x02},
    {0x3E, 0x51, 0x49, 0x45, 0x3E},
    {0x00, 0x42, 0x7F, 0x40, 0x00},
    {0x42, 0x61, 0x51, 0x49, 0x46},
    {0x21, 0x41, 0x45, 0x4B, 0x31},
    {0x18, 0x14, 0x12, 0x7F, 0x10},
    {0x27, 0x45, 0x45, 0x45, 0x39},
    {0x3C, 0x4A, 0x49, 0x49, 0x31},
    {0x41, 0x21, 0x11, 0x09, 0x07},
    {0x36, 0x49, 0x49, 0x49, 0x36},
    {0x46, 0x49, 0x49, 0x29, 0x1E},
    {0x00, 0x36, 0x36, 0x00, 0x00},
    {0x00, 0x56, 0x36, 0x00, 0x00},
    {0x08, 0x14, 0x22, 0x41, 0x00},
    {0x14, 0x14, 0x14, 0x14, 0x14},
    {0x00, 0x41, 0x22, 0x14, 0x08},
    {0x02, 0x01, 0x51, 0x09, 0x06},
    {0x32, 0x49, 0x79, 0x41, 0x3E},
    {0x7E, 0x11, 0x11, 0x11, 0x7E},
    {0x7F, 0x49, 0x49, 0x49, 0x36},
    {0x3E, 0x41, 0x41, 0x41, 0x22},
    {0x7F, 0x41, 0x41, 0x22, 0x1C},
    {0x7F, 0x49, 0x49, 0x49, 0x41},
    {0x7F, 0x09, 0x09, 0x09, 0x01},
    {0x3E, 0x41, 0x49, 0x49, 0x7A},
    {0x7F, 0x08, 0x08, 0x08, 0x7F},
    {0x00, 0x41, 0x7F, 0x41, 0x00},
    {0x20, 0x40, 0x41, 0x3F, 0x01},
    {0x7F, 0x08, 0x14, 0x22, 0x41},
    {0x7F, 0x40, 0x40, 0x40, 0x40},
    {0x7F, 0x02, 0x0C, 0x02, 0x7F},
    {0x7F, 0x04, 0x08, 0x10, 0x7F},
    {0x3E, 0x41, 0x41, 0x41, 0x3E},
    {0x7F, 0x09, 0x09, 0x09, 0x06},
    {0x3E, 0x41, 0x51, 0x21, 0x5E},
    {0x7F, 0x09, 0x19, 0x29, 0x46},
    {0x46, 0x49, 0x49, 0x49, 0x31},
    {0x01, 0x01, 0x7F, 0x01, 0x01},
    {0x3F, 0x40, 0x40, 0x40, 0x3F},
    {0x1F, 0x20, 0x40, 0x20, 0x1F},
    {0x3F, 0x40, 0x38, 0x40, 0x3F},
    {0x63, 0x14, 0x08, 0x14, 0x63},
    {0x07, 0x08, 0x70, 0x08, 0x07},
    {0x61, 0x51, 0x49, 0x45, 0x43},
    {0x00, 0x7F, 0x41, 0x41, 0x00},
    {0x02, 0x04, 0x08, 0x10, 0x20},
    {0x00, 0x41, 0x41, 0x7F, 0x00},
    {0x04, 0x02, 0x01, 0x02, 0x04},
    {0x40, 0x40, 0x40, 0x40, 0x40},
    {0x00, 0x01, 0x02, 0x04, 0x00},
    {0x20, 0x54, 0x54, 0x54, 0x78},
    {0x7F, 0x48, 0x44, 0x44, 0x38},
    {0x38, 0x44, 0x44, 0x44, 0x20},
    {0x38, 0x44, 0x44, 0x48, 0x7F},
    {0x38, 0x54, 0x54, 0x54, 0x18},
    {0x08, 0x7E, 0x09, 0x01, 0x02},
    {0x18, 0xA4, 0xA4, 0xA4, 0x7C},
    {0x7F, 0x08, 0x04, 0x04, 0x78},
    {0x00, 0x44, 0x7D, 0x40, 0x00},
    {0x40, 0x80, 0x84, 0x7D, 0x00},
    {0x7F, 0x10, 0x28, 0x44, 0x00},
    {0x00, 0x41, 0x7F, 0x40, 0x00},
    {0x7C, 0x04, 0x18, 0x04, 0x78},
    {0x7C, 0x08, 0x04, 0x04, 0x78},
    {0x38, 0x44, 0x44, 0x44, 0x38},
    {0xFC, 0x24, 0x24, 0x24, 0x18},
    {0x18, 0x24, 0x24, 0x18, 0xFC},
    {0x7C, 0x08, 0x04, 0x04, 0x08},
    {0x48, 0x54, 0x54, 0x54, 0x20},
    {0x04, 0x3F, 0x44, 0x40, 0x20},
    {0x3C, 0x40, 0x40, 0x20, 0x7C},
    {0x1C, 0x20, 0x40, 0x20, 0x1C},
    {0x3C, 0x40, 0x30, 0x40, 0x3C},
    {0x44, 0x28, 0x10, 0x28, 0x44},
    {0x1C, 0xA0, 0xA0, 0xA0, 0x7C},
    {0x44, 0x64, 0x54, 0x4C, 0x44},
    {0x00, 0x08, 0x36, 0x41, 0x00},
    {0x00, 0x00, 0x7F, 0x00, 0x00},
    {0x00, 0x41, 0x36, 0x08, 0x00},
    {0x08, 0x04, 0x08, 0x10, 0x08},
    {0x78, 0x46, 0x41, 0x46, 0x78}
};

static GPIO_TypeDef* GetCsPort(uint8_t displayId)
{
    return (displayId == DISPLAY_LANE1) ? CS1_GPIO_Port : CS2_GPIO_Port;
}

static uint16_t GetCsPin(uint8_t displayId)
{
    return (displayId == DISPLAY_LANE1) ? CS1_Pin : CS2_Pin;
}

static GPIO_TypeDef* GetDcPort(uint8_t displayId)
{
    return (displayId == DISPLAY_LANE1) ? DC1_GPIO_Port : DC2_GPIO_Port;
}

static uint16_t GetDcPin(uint8_t displayId)
{
    return (displayId == DISPLAY_LANE1) ? DC1_Pin : DC2_Pin;
}

static GPIO_TypeDef* GetResetPort(uint8_t displayId)
{
    return (displayId == DISPLAY_LANE1) ? RESET1_GPIO_Port : RESET2_GPIO_Port;
}

static uint16_t GetResetPin(uint8_t displayId)
{
    return (displayId == DISPLAY_LANE1) ? RESET1_Pin : RESET2_Pin;
}

static GPIO_TypeDef* GetBlPort(uint8_t displayId)
{
    return (displayId == DISPLAY_LANE1) ? SPI1_BL_GPIO_Port : SPI2_BL_GPIO_Port;
}

static uint16_t GetBlPin(uint8_t displayId)
{
    return (displayId == DISPLAY_LANE1) ? SPI1_BL_Pin : SPI2_BL_Pin;
}

static SPI_HandleTypeDef* GetSpiHandle(uint8_t displayId)
{
    return (displayId == DISPLAY_LANE1) ? &hspi1 : &hspi2;
}

static void Display_CS(uint8_t displayId, uint8_t state)
{
    HAL_GPIO_WritePin(GetCsPort(displayId), GetCsPin(displayId), 
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void Display_DC(uint8_t displayId, uint8_t state)
{
    HAL_GPIO_WritePin(GetDcPort(displayId), GetDcPin(displayId), 
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void Display_RST(uint8_t displayId, uint8_t state)
{
    HAL_GPIO_WritePin(GetResetPort(displayId), GetResetPin(displayId), 
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void Display_WriteCommand(uint8_t displayId, uint8_t cmd)
{
    Display_CS(displayId, 1);
    Display_DC(displayId, 0);
    Display_CS(displayId, 0);
    
    HAL_SPI_Transmit(GetSpiHandle(displayId), &cmd, 1, HAL_MAX_DELAY);
    
    Display_CS(displayId, 1);
}

static void Display_WriteData(uint8_t displayId, uint8_t data)
{
    Display_CS(displayId, 1);
    Display_DC(displayId, 1);
    Display_CS(displayId, 0);
    
    HAL_SPI_Transmit(GetSpiHandle(displayId), &data, 1, HAL_MAX_DELAY);
    
    Display_CS(displayId, 1);
}

static void Display_WriteData16(uint8_t displayId, uint16_t data)
{
    Display_CS(displayId, 1);
    Display_DC(displayId, 1);
    Display_CS(displayId, 0);
    
    uint8_t buf[2] = {data >> 8, data & 0xFF};
    HAL_SPI_Transmit(GetSpiHandle(displayId), buf, 2, HAL_MAX_DELAY);
    
    Display_CS(displayId, 1);
}

static void Display_SetWindow(uint8_t displayId, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    Display_WriteCommand(displayId, 0x2A);
    Display_WriteData16(displayId, x1);
    Display_WriteData16(displayId, x2);
    
    Display_WriteCommand(displayId, 0x2B);
    Display_WriteData16(displayId, y1);
    Display_WriteData16(displayId, y2);
    
    Display_WriteCommand(displayId, 0x2C);
}

void Display_Init(void)
{
    // 简化初始化过程，减少代码大小
    Display_SetBacklight(DISPLAY_LANE1, 1);
    Display_SetBacklight(DISPLAY_LANE2, 1);
    
    Display_InitScreen(DISPLAY_LANE1);
    Display_InitScreen(DISPLAY_LANE2);
    
    Display_Clear(DISPLAY_LANE1, BLACK);
    Display_Clear(DISPLAY_LANE2, BLACK);
}

void Display_InitScreen(uint8_t displayId)
{
    Display_CS(displayId, 1);
    Display_RST(displayId, 0);
    HAL_Delay(50);
    Display_RST(displayId, 1);
    HAL_Delay(150);
    
    // 软复位
    Display_WriteCommand(displayId, 0x01);
    HAL_Delay(150);
    
    // 睡眠退出
    Display_WriteCommand(displayId, 0x11);
    HAL_Delay(120);
    
    // 颜色模式设置
    Display_WriteCommand(displayId, 0x3A);
    Display_WriteData(displayId, 0x55); // RGB565格式
    
    // 内存访问控制 - 设置为RGB格式和横向显示
    Display_WriteCommand(displayId, 0x36);
    Display_WriteData(displayId, 0x08); // RGB格式，横向显示
    
    // 行列设置
    Display_WriteCommand(displayId, 0x2A); // 列地址设置
    Display_WriteData(displayId, 0x00);
    Display_WriteData(displayId, 0x00);
    Display_WriteData(displayId, 0x00);
    Display_WriteData(displayId, 0xEF); // 240列
    
    Display_WriteCommand(displayId, 0x2B); // 行地址设置
    Display_WriteData(displayId, 0x00);
    Display_WriteData(displayId, 0x00);
    Display_WriteData(displayId, 0x00);
    Display_WriteData(displayId, 0xEF); // 240行
    
    // 显示开
    Display_WriteCommand(displayId, 0x29);
    HAL_Delay(50);
    
    // 亮度设置
    Display_WriteCommand(displayId, 0x51); // 亮度控制
    Display_WriteData(displayId, 0xFF); // 最大亮度
    HAL_Delay(100);
}

void Display_SetBacklight(uint8_t displayId, uint8_t state)
{
    HAL_GPIO_WritePin(GetBlPort(displayId), GetBlPin(displayId), 
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Display_Clear(uint8_t displayId, uint16_t color)
{
    Display_SetWindow(displayId, 0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);
    
    Display_CS(displayId, 1);
    Display_DC(displayId, 1);
    Display_CS(displayId, 0);
    
    // 直接传输数据，减少代码大小
    uint8_t high = color >> 8;
    uint8_t low = color & 0xFF;
    uint8_t buf[2] = {high, low};
    
    for (int i = 0; i < ST7789_WIDTH * ST7789_HEIGHT; i++) {
        HAL_SPI_Transmit(GetSpiHandle(displayId), buf, 2, 100);
    }
    
    Display_CS(displayId, 1);
}

void Display_FillRect(uint8_t displayId, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    Display_SetWindow(displayId, x, y, x + w - 1, y + h - 1);
    
    Display_CS(displayId, 1);
    Display_DC(displayId, 1);
    Display_CS(displayId, 0);
    
    // 直接传输数据，减少代码大小
    uint8_t high = color >> 8;
    uint8_t low = color & 0xFF;
    uint8_t buf[2] = {high, low};
    
    for (int i = 0; i < w * h; i++) {
        HAL_SPI_Transmit(GetSpiHandle(displayId), buf, 2, 100);
    }
    
    Display_CS(displayId, 1);
}

// 简单的数字显示函数，减少代码大小
void Display_DrawDigit(uint8_t displayId, uint16_t x, uint16_t y, uint8_t digit, uint16_t color, uint8_t size)
{
    // 简单的数字显示，只显示0-9
    if (digit > 9) return;
    
    // 定义简单的数字点阵
    const uint8_t digits[10][5] = {
        {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 0
        {0x00, 0x00, 0x21, 0x7F, 0x01}, // 1
        {0x21, 0x43, 0x45, 0x49, 0x31}, // 2
        {0x22, 0x41, 0x49, 0x49, 0x36}, // 3
        {0x08, 0x0C, 0x14, 0x7F, 0x04}, // 4
        {0x72, 0x51, 0x51, 0x51, 0x4E}, // 5
        {0x3E, 0x49, 0x49, 0x49, 0x26}, // 6
        {0x40, 0x40, 0x40, 0x4F, 0x70}, // 7
        {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
        {0x3E, 0x49, 0x49, 0x49, 0x32}  // 9
    };
    
    const uint8_t *p = digits[digit];
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 8; j++) {
            if (*p & (1 << (7 - j))) {
                Display_FillRect(displayId, x + j * size, y + i * size, size, size, color);
            }
        }
        p++;
    }
}

// 绘制单个字符
void Display_DrawChar(uint8_t displayId, uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bgcolor, uint8_t size)
{
    if (c < 32 || c > 127) return;
    
    const uint8_t *p = font5x8[c - 32];
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 8; j++) {
            if (*p & (1 << (7 - j))) {
                Display_FillRect(displayId, x + j * size, y + i * size, size, size, color);
            } else {
                Display_FillRect(displayId, x + j * size, y + i * size, size, size, bgcolor);
            }
        }
        p++;
    }
}

// 绘制单个字符（旋转90度，正确方向）
void Display_DrawCharRotated(uint8_t displayId, uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bgcolor, uint8_t size)
{
    if (c < 32 || c > 127) return;
    
    const uint8_t *p = font5x8[c - 32];
    
    for (int j = 0; j < 8; j++) {
        for (int i = 0; i < 5; i++) {
            if (p[i] & (1 << (7 - j))) {
                Display_FillRect(displayId, x + j * size, y + i * size, size, size, color);
            } else {
                Display_FillRect(displayId, x + j * size, y + i * size, size, size, bgcolor);
            }
        }
    }
}

// 绘制字符串
void Display_DrawString(uint8_t displayId, uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bgcolor, uint8_t size)
{
    while (*str) {
        Display_DrawChar(displayId, x, y, *str++, color, bgcolor, size);
        x += 8 * size;
    }
}

// 绘制字符串（旋转90度，字符横着）
void Display_DrawStringRotated(uint8_t displayId, uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bgcolor, uint8_t size)
{
    while (*str) {
        Display_DrawCharRotated(displayId, x, y, *str++, color, bgcolor, size);
        y += 8 * size;
    }
}

// 绘制垂直字符串（从上到下，单词整体显示）
void Display_DrawVerticalString(uint8_t displayId, uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bgcolor, uint8_t size)
{
    int i = 0;
    while (*str) {
        Display_DrawChar(displayId, x, y + i * 10 * size, *str++, color, bgcolor, size);
        i++;
    }
}

void Display_Update(uint8_t displayId)
{
    static uint8_t lastSafeSpeed[2] = {0xFF, 0xFF};
    static uint8_t lastAlertLevel[2] = {0xFF, 0xFF};
    static VehicleType_t lastVehicleType[2] = {0xFF, 0xFF};
    
    uint8_t safeSpeed = (displayId == DISPLAY_LANE1) ? g_decisionResult.safeSpeed1 : g_decisionResult.safeSpeed2;
    uint8_t alertLevel = (displayId == DISPLAY_LANE1) ? g_decisionResult.alertLevel1 : g_decisionResult.alertLevel2;
    VehicleType_t vehicleType = (displayId == DISPLAY_LANE1) ? g_lane1Status.vehicleType : g_lane2Status.vehicleType;
    
    if (lastSafeSpeed[displayId] == safeSpeed && 
        lastAlertLevel[displayId] == alertLevel && 
        lastVehicleType[displayId] == vehicleType) {
        return;
    }
    
    lastSafeSpeed[displayId] = safeSpeed;
    lastAlertLevel[displayId] = alertLevel;
    lastVehicleType[displayId] = vehicleType;
    
    Display_FillRect(displayId, 0, 0, ST7789_WIDTH, ST7789_HEIGHT, BLACK);
    
    // 安全状态
    uint16_t alertColor;
    const char *alertStr;
    switch (alertLevel) {
        case ALERT_LEVEL_SAFE:
            alertColor = GREEN;
            alertStr = "SAFE";
            break;
        case ALERT_LEVEL_NORMAL:
            alertColor = BLUE;
            alertStr = "NORMAL";
            break;
        case ALERT_LEVEL_WARNING:
            alertColor = RED;
            alertStr = "WARNING";
            break;
        case ALERT_LEVEL_DANGER:
            alertColor = YELLOW;
            alertStr = "DANGER";
            break;
        default:
            alertColor = WHITE;
            alertStr = "SAFE";
            break;
    }
    // 绘制颜色方块
    Display_FillRect(displayId, 200, 0, 40, 240, alertColor);
    // 显示安全状态文字
    Display_DrawVerticalString(displayId, 210, 10, alertStr, BLACK, alertColor, FONT_SIZE_NORMAL);
    
    // 第一排：车道号（垂直显示）
    char laneStr[10];
    sprintf(laneStr, "L%d", displayId + 1);
    Display_DrawVerticalString(displayId, 30, 10, laneStr, WHITE, BLACK, FONT_SIZE_NORMAL);
    
    // 第二排：车辆类型（垂直显示）
    const char *vehicleStr;
    switch (vehicleType) {
        case VEHICLE_NONE:
            vehicleStr = "None";
            break;
        case VEHICLE_SMALL:
            vehicleStr = "Small";
            break;
        case VEHICLE_LARGE:
            vehicleStr = "Large";
            break;
        default:
            vehicleStr = "None";
            break;
    }
    Display_DrawVerticalString(displayId, 70, 10, vehicleStr, WHITE, BLACK, FONT_SIZE_NORMAL);
    
    // 第三排：速度（水平显示，字符横着）
    char speedStr[20];
    sprintf(speedStr, "%dkm/h", safeSpeed);
    Display_DrawStringRotated(displayId, 140, 10, speedStr, WHITE, BLACK, FONT_SIZE_NORMAL);
}
