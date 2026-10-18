#include "oled_display.h"
#include "stm32f1xx_hal.h"
#include "encoder_reader.h"
#include "rehabilitation_mode.h"
#include "motor_controller.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

OLED_State_t g_oled_state = {
    .hi2c = NULL,
    .address = OLED_I2C_ADDRESS,
    .initialized = 0,
    .inverted = 0
};

// 字体数组前向声明
static const uint8_t s_font6x8[95][6];

// 私有函数声明
static void OLED_Write_Command(uint8_t cmd);
static void OLED_Write_Data(uint8_t data);
static void OLED_Set_Position(uint8_t x, uint8_t y);

/**
 * @brief 向OLED写入命令
 */
static void OLED_Write_Command(uint8_t cmd)
{
    if (g_oled_state.hi2c == NULL) return;
    
    uint8_t data[2] = {0x00, cmd};
    HAL_I2C_Master_Transmit(g_oled_state.hi2c, g_oled_state.address, data, 2, HAL_MAX_DELAY);
}

/**
 * @brief 向OLED写入数据
 */
static void OLED_Write_Data(uint8_t data)
{
    if (g_oled_state.hi2c == NULL) return;
    
    uint8_t buf[2] = {0x40, data};
    HAL_I2C_Master_Transmit(g_oled_state.hi2c, g_oled_state.address, buf, 2, HAL_MAX_DELAY);
}

/**
 * @brief 设置OLED显示位置
 */
static void OLED_Set_Position(uint8_t x, uint8_t y)
{
    OLED_Write_Command(0xB0 + y);
    OLED_Write_Command(((x & 0xF0) >> 4) | 0x10);
    OLED_Write_Command(x & 0x0F);
}

/**
 * @brief OLED初始化
 */
void OLED_Init(I2C_HandleTypeDef* hi2c, uint8_t address)
{
    g_oled_state.hi2c = hi2c;
    g_oled_state.address = address;
    g_oled_state.initialized = 0;
    
    HAL_Delay(100);
    
    OLED_Write_Command(0xAE); // 关闭显示
    OLED_Write_Command(0xD5); // 设置时钟分频因子,震荡频率
    OLED_Write_Command(0x80); // [3:0],分频因子;[7:4],震荡频率
    OLED_Write_Command(0xA8); // 设置驱动路数
    OLED_Write_Command(0x3F); // 0x3F(1/64) 
    OLED_Write_Command(0xD3); // 设置显示偏移
    OLED_Write_Command(0x00); // 无偏移
    OLED_Write_Command(0x40); // 设置显示开始行 [5:0],行数
    OLED_Write_Command(0x8D); // 电荷泵设置
    OLED_Write_Command(0x14); // 开启电荷泵
    OLED_Write_Command(0x20); // 设置内存地址模式
    OLED_Write_Command(0x02); // 页地址模式
    OLED_Write_Command(0xA1); // 段重定义设置,bit0:0,0->0;1,0->127
    OLED_Write_Command(0xC8); // COM扫描方向;bit3:0,普通模式;1,重定义模式
    OLED_Write_Command(0xDA); // 设置COM硬件引脚配置
    OLED_Write_Command(0x12);
    OLED_Write_Command(0x81); // 对比度设置
    OLED_Write_Command(0xCF); // 1~255;默认0x7F (亮度设置,越大越亮)
    OLED_Write_Command(0xD9); // 设置预充电周期
    OLED_Write_Command(0xF1); // [3:0],PHASE 1;[7:4],PHASE 2;
    OLED_Write_Command(0xDB); // 设置VCOMH取消选择级别
    OLED_Write_Command(0x30); // 0x20,0.77xVcc
    OLED_Write_Command(0xA4); // 全局显示开启;bit0:1,开启;0,关闭
    OLED_Write_Command(0xA6); // 设置显示方式;bit0:1,反相显示;0,正常显示
    OLED_Write_Command(0xAF); // 开启显示
    
    OLED_Clear();
    g_oled_state.initialized = 1;
}

/**
 * @brief 清屏
 */
void OLED_Clear(void)
{
    uint8_t i;
    for (i = 0; i < 8; i++) {
        OLED_Write_Command(0xB0 + i);
        OLED_Write_Command(0x00);
        OLED_Write_Command(0x10);
        for (uint8_t j = 0; j < 128; j++) {
            OLED_Write_Data(0x00);
        }
    }
}

/**
 * @brief 刷新显示
 */
void OLED_Refresh(void)
{
    OLED_Write_Command(0xAF);
}

/**
 * @brief 绘制点
 */
void OLED_Draw_Pixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= 128 || y >= 64) return;
    
    uint8_t page = y / 8;
    uint8_t bit = y % 8;
    
    OLED_Set_Position(x, page);
    if (color) {
        OLED_Write_Data(1 << bit);
    } else {
        OLED_Write_Data(0);
    }
}

/**
 * @brief 绘制字符串
 */
void OLED_Draw_String(uint8_t x, uint8_t y, const char* str, const uint8_t* font, uint8_t font_width, uint8_t font_height, uint8_t invert)
{
    if (str == NULL || font == NULL) return;
    
    uint8_t i;
    while (*str) {
        if (*str < 32 || *str > 126) {
            str++;
            continue;
        }
        
        const uint8_t* char_data = &font[(*str - 32) * font_width];
        for (i = 0; i < font_width; i++) {
            uint8_t data = char_data[i];
            if (invert) data = ~data;
            
            OLED_Set_Position(x + i, y / 8);
            OLED_Write_Data(data);
        }
        x += font_width;
        str++;
    }
}

/**
 * @brief 显示系统状态
 */
void OLED_Display_System_Status(void)
{
    float position_mm = Encoder_Get_Position_MM();
    float velocity_mm_s = Encoder_Get_Velocity_MM_S();
    uint8_t speed_percent = Motor_Get_Speed();
    uint8_t direction = Motor_Get_Direction();
    uint8_t rehab_active = Rehabilitation_Is_Active();
    
    OLED_Clear();
    
    char buf[32];
    sprintf(buf, "Pos: %.1fmm", position_mm);
    OLED_Draw_String(0, 0, buf, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    sprintf(buf, "Vel: %.1fmm/s", velocity_mm_s);
    OLED_Draw_String(0, 16, buf, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    sprintf(buf, "Speed: %d%%", speed_percent);
    OLED_Draw_String(0, 32, buf, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    sprintf(buf, "Rehab: %s", rehab_active ? "ON" : "OFF");
    OLED_Draw_String(0, 48, buf, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    OLED_Refresh();
}

/**
 * @brief 显示康复模式状态
 */
void OLED_Display_Rehab_Status(void)
{
    uint8_t level = Rehabilitation_Get_Level();
    uint32_t reps = Rehabilitation_Get_Repetitions();
    uint32_t time = Rehabilitation_Get_Training_Time();
    float min_pos = Rehabilitation_Get_Min_Position();
    float max_pos = Rehabilitation_Get_Max_Position();
    float current_pos = Encoder_Get_Position_MM();
    
    OLED_Clear();
    
    char buf[32];
    sprintf(buf, "Rehab Mode Lv%d", level);
    OLED_Draw_String(0, 0, buf, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    sprintf(buf, "Reps: %ld", reps);
    OLED_Draw_String(0, 16, buf, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    sprintf(buf, "Time: %lds", time);
    OLED_Draw_String(0, 32, buf, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    sprintf(buf, "Range: %.0f-%.0fmm", min_pos, max_pos);
    OLED_Draw_String(0, 48, buf, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    sprintf(buf, "Pos: %.1fmm", current_pos);
    OLED_Draw_String(0, 56, buf, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    OLED_Refresh();
}

/**
 * @brief 安全初始化OLED
 */
void OLED_Init_Safe(void)
{
    OLED_Init(&hi2c1, OLED_I2C_ADDRESS);
}

/**
 * @brief 安全清屏
 */
void OLED_Clear_Safe(void)
{
    OLED_Clear();
}

/**
 * @brief 安全刷新
 */
void OLED_Refresh_Safe(void)
{
    OLED_Refresh();
}

/**
 * @brief 安全显示字符串
 */
void OLED_Show_String(uint8_t x, uint8_t y, const char* str, uint8_t size)
{
    OLED_Draw_String(x, y, str, (const uint8_t*)s_font6x8, 6, 8, 1);
}

/**
 * @brief 显示系统状态
 */
void OLED_Display_System_Info(void)
{
    OLED_Clear();
    
    // 显示标题
    OLED_Draw_String(0, 0, "Arm Rehab System", (const uint8_t*)s_font6x8, 6, 8, 1);
    
    // 显示电机状态
    char motor_status[32];
    MotorDirection_t dir = Motor_Get_Direction();
    uint8_t speed = Motor_Get_Speed();
    
    if (dir == MOTOR_DIR_STOP) {
        sprintf(motor_status, "Motor: STOP");
    } else if (dir == MOTOR_DIR_FORWARD) {
        sprintf(motor_status, "Motor: FWD %d%%", speed);
    } else {
        sprintf(motor_status, "Motor: REV %d%%", speed);
    }
    OLED_Draw_String(0, 16, motor_status, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    // 显示编码器计数
    char encoder_str[32];
    int32_t count = Encoder_Get_Count();
    sprintf(encoder_str, "Encoder: %ld", count);
    OLED_Draw_String(0, 24, encoder_str, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    // 显示位置
    char pos_str[32];
    float pos_mm = Encoder_Get_Position_MM();
    sprintf(pos_str, "Pos: %.1fmm", pos_mm);
    OLED_Draw_String(0, 32, pos_str, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    // 显示康复模式状态
    char mode_str[32];
    if (Rehabilitation_Is_Active()) {
        sprintf(mode_str, "Rehab: ON Lv%d", Rehabilitation_Get_Level());
    } else {
        sprintf(mode_str, "Rehab: OFF");
    }
    OLED_Draw_String(0, 40, mode_str, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    // 显示提示信息
    OLED_Draw_String(0, 56, "Ready", (const uint8_t*)s_font6x8, 6, 8, 1);
    
    OLED_Refresh();
}

/**
 * @brief 显示康复模式详细信息
 */
void OLED_Display_Rehab_Details(void)
{
    OLED_Clear();
    
    // 显示标题
    OLED_Draw_String(0, 0, "Rehabilitation Mode", (const uint8_t*)s_font6x8, 6, 8, 1);
    
    // 显示当前模式
    char mode_str[32];
    uint8_t level = Rehabilitation_Get_Level();
    switch (level) {
        case 1: sprintf(mode_str, "Mode: Gentle"); break;
        case 2: case 3: sprintf(mode_str, "Mode: Easy"); break;
        case 4: case 5: case 6: sprintf(mode_str, "Mode: Moderate"); break;
        case 7: case 8: sprintf(mode_str, "Mode: Hard"); break;
        case 9: case 10: sprintf(mode_str, "Mode: Intense"); break;
        default: sprintf(mode_str, "Mode: Level %d", level);
    }
    OLED_Draw_String(0, 16, mode_str, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    // 显示阻力等级
    char level_str[32];
    sprintf(level_str, "Resistance: Lv%d", level);
    OLED_Draw_String(0, 24, level_str, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    // 显示重复次数
    char reps_str[32];
    uint32_t reps = Rehabilitation_Get_Repetitions();
    sprintf(reps_str, "Reps: %ld", reps);
    OLED_Draw_String(0, 32, reps_str, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    // 显示训练时间
    char time_str[32];
    uint32_t time = Rehabilitation_Get_Training_Time();
    sprintf(time_str, "Time: %lds", time);
    OLED_Draw_String(0, 40, time_str, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    // 显示位置范围
    char range_str[32];
    float min_pos = Rehabilitation_Get_Min_Position();
    float max_pos = Rehabilitation_Get_Max_Position();
    sprintf(range_str, "Range: %.0f-%.0fmm", min_pos, max_pos);
    OLED_Draw_String(0, 48, range_str, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    // 显示当前位置
    char pos_str[32];
    float pos_mm = Encoder_Get_Position_MM();
    sprintf(pos_str, "Pos: %.1fmm", pos_mm);
    OLED_Draw_String(0, 56, pos_str, (const uint8_t*)s_font6x8, 6, 8, 1);
    
    OLED_Refresh();
}

// 6x8 ASCII字体数组
static const uint8_t s_font6x8[95][6] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 空格
    {0x00, 0x00, 0x5f, 0x00, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00, 0x00}, // "
    {0x14, 0x7f, 0x14, 0x7f, 0x14, 0x00}, // #
    {0x24, 0x2a, 0x7f, 0x2a, 0x12, 0x00}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62, 0x00}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50, 0x00}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00, 0x00}, // '
    {0x00, 0x1c, 0x22, 0x41, 0x00, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1c, 0x00, 0x00}, // )
    {0x14, 0x08, 0x3e, 0x08, 0x14, 0x00}, // *
    {0x08, 0x08, 0x3e, 0x08, 0x08, 0x00}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08, 0x00}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02, 0x00}, // /
    {0x3e, 0x51, 0x49, 0x45, 0x3e, 0x00}, // 0
    {0x00, 0x42, 0x7f, 0x40, 0x00, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46, 0x00}, // 2
    {0x21, 0x41, 0x45, 0x4b, 0x31, 0x00}, // 3
    {0x18, 0x14, 0x12, 0x7f, 0x10, 0x00}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39, 0x00}, // 5
    {0x3c, 0x4a, 0x49, 0x49, 0x30, 0x00}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03, 0x00}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36, 0x00}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1e, 0x00}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14, 0x00}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08, 0x00}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06, 0x00}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3e, 0x00}, // @
    {0x7e, 0x11, 0x11, 0x11, 0x7e, 0x00}, // A
    {0x7f, 0x49, 0x49, 0x49, 0x36, 0x00}, // B
    {0x3e, 0x41, 0x41, 0x41, 0x22, 0x00}, // C
    {0x7f, 0x41, 0x41, 0x41, 0x3e, 0x00}, // D
    {0x7f, 0x49, 0x49, 0x49, 0x41, 0x00}, // E
    {0x7f, 0x09, 0x09, 0x09, 0x01, 0x00}, // F
    {0x3e, 0x41, 0x49, 0x49, 0x7a, 0x00}, // G
    {0x7f, 0x08, 0x08, 0x08, 0x7f, 0x00}, // H
    {0x00, 0x41, 0x7f, 0x41, 0x00, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3f, 0x01, 0x00}, // J
    {0x7f, 0x08, 0x14, 0x22, 0x41, 0x00}, // K
    {0x7f, 0x40, 0x40, 0x40, 0x40, 0x00}, // L
    {0x7f, 0x02, 0x04, 0x02, 0x7f, 0x00}, // M
    {0x7f, 0x04, 0x08, 0x10, 0x7f, 0x00}, // N
    {0x3e, 0x41, 0x41, 0x41, 0x3e, 0x00}, // O
    {0x7f, 0x09, 0x09, 0x09, 0x06, 0x00}, // P
    {0x3e, 0x41, 0x51, 0x21, 0x5e, 0x00}, // Q
    {0x7f, 0x09, 0x19, 0x29, 0x46, 0x00}, // R
    {0x26, 0x49, 0x49, 0x49, 0x32, 0x00}, // S
    {0x01, 0x01, 0x7f, 0x01, 0x01, 0x00}, // T
    {0x3f, 0x40, 0x40, 0x40, 0x3f, 0x00}, // U
    {0x1f, 0x20, 0x40, 0x20, 0x1f, 0x00}, // V
    {0x3f, 0x40, 0x38, 0x40, 0x3f, 0x00}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63, 0x00}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07, 0x00}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43, 0x00}, // Z
    {0x00, 0x7f, 0x41, 0x41, 0x00, 0x00}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20, 0x00}, // \
    {0x00, 0x41, 0x41, 0x7f, 0x00, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04, 0x00}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40, 0x00}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78, 0x00}, // a
    {0x7f, 0x48, 0x44, 0x44, 0x38, 0x00}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20, 0x00}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7f, 0x00}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18, 0x00}, // e
    {0x08, 0x7e, 0x09, 0x01, 0x02, 0x00}, // f
    {0x0c, 0x52, 0x52, 0x52, 0x3e, 0x00}, // g
    {0x7f, 0x08, 0x04, 0x04, 0x78, 0x00}, // h
    {0x00, 0x44, 0x7d, 0x40, 0x00, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3d, 0x00, 0x00}, // j
    {0x7f, 0x10, 0x28, 0x44, 0x00, 0x00}, // k
    {0x00, 0x41, 0x7f, 0x40, 0x00, 0x00}, // l
    {0x7c, 0x04, 0x18, 0x04, 0x78, 0x00}, // m
    {0x7c, 0x08, 0x04, 0x04, 0x78, 0x00}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38, 0x00}, // o
    {0x7c, 0x14, 0x14, 0x14, 0x08, 0x00}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7c, 0x00}, // q
    {0x7c, 0x08, 0x04, 0x04, 0x08, 0x00}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20, 0x00}, // s
    {0x04, 0x3f, 0x44, 0x40, 0x20, 0x00}, // t
    {0x3c, 0x40, 0x40, 0x20, 0x7c, 0x00}, // u
    {0x1c, 0x20, 0x40, 0x20, 0x1c, 0x00}, // v
    {0x3c, 0x40, 0x30, 0x40, 0x3c, 0x00}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44, 0x00}, // x
    {0x0c, 0x50, 0x50, 0x50, 0x3c, 0x00}, // y
    {0x44, 0x64, 0x54, 0x4c, 0x44, 0x00}, // z
    {0x00, 0x08, 0x36, 0x41, 0x00, 0x00}, // {
    {0x00, 0x00, 0x7f, 0x00, 0x00, 0x00}, // |
    {0x00, 0x41, 0x36, 0x08, 0x00, 0x00}, // }
    {0x08, 0x08, 0x2a, 0x1c, 0x08, 0x00}, // ~
};
