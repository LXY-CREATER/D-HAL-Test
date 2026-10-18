// 在oled.c顶部，删除有问题的声明
#include "oled.h"
#include "all_includes.h"
#include <stdlib.h>
// OLED显存
static uint8_t OLED_GRAM[128][8];

// 在 oled.c 文件开头添加全局状态结构体定义

// 系统状态结构体（临时定义）
typedef struct {
    int motor_speed;
    int resistance_level;
    long encoder_position;
    int encoder_speed;
    uint32_t runtime_ms;
} SystemStatus_Temp;

// 全局系统状态变量（临时）
SystemStatus_Temp system_status = {
    .motor_speed = 0,
    .resistance_level = 1,
    .encoder_position = 0,
    .encoder_speed = 0,
    .runtime_ms = 0
};

// 更新函数（在main中调用）
void Update_System_Status(int speed, int level, long position, int enc_speed)
{
    system_status.motor_speed = speed;
    system_status.resistance_level = level;
    system_status.encoder_position = position;
    system_status.encoder_speed = enc_speed;
    system_status.runtime_ms = HAL_GetTick();
}

// OLED I2C地址
#define OLED_ADDRESS 0x78  // 通常为0x78或0x7A

// 完整的6x8 ASCII字符集
static const uint8_t ASCII_6x8[][6] = {
    {0x00,0x00,0x00,0x00,0x00,0x00}, // 空格  
    {0x00,0x00,0x5F,0x00,0x00,0x00}, // !
    {0x00,0x07,0x00,0x07,0x00,0x00}, // "
    {0x14,0x7F,0x14,0x7F,0x14,0x00}, // #
    {0x24,0x2A,0x7F,0x2A,0x12,0x00}, // $
    {0x23,0x13,0x08,0x64,0x62,0x00}, // %
    {0x36,0x49,0x55,0x22,0x50,0x00}, // &
    {0x00,0x05,0x03,0x00,0x00,0x00}, // '
    {0x00,0x1C,0x22,0x41,0x00,0x00}, // (
    {0x00,0x41,0x22,0x1C,0x00,0x00}, // )
    {0x14,0x08,0x3E,0x08,0x14,0x00}, // *
    {0x08,0x08,0x3E,0x08,0x08,0x00}, // +
    {0x00,0x50,0x30,0x00,0x00,0x00}, // ,
    {0x08,0x08,0x08,0x08,0x08,0x00}, // -
    {0x00,0x60,0x60,0x00,0x00,0x00}, // .
    {0x20,0x10,0x08,0x04,0x02,0x00}, // /
    {0x3E,0x51,0x49,0x45,0x3E,0x00}, // 0
    {0x00,0x42,0x7F,0x40,0x00,0x00}, // 1
    {0x42,0x61,0x51,0x49,0x46,0x00}, // 2
    {0x21,0x41,0x45,0x4B,0x31,0x00}, // 3
    {0x18,0x14,0x12,0x7F,0x10,0x00}, // 4
    {0x27,0x45,0x45,0x45,0x39,0x00}, // 5
    {0x3C,0x4A,0x49,0x49,0x30,0x00}, // 6
    {0x01,0x71,0x09,0x05,0x03,0x00}, // 7
    {0x36,0x49,0x49,0x49,0x36,0x00}, // 8
    {0x06,0x49,0x49,0x29,0x1E,0x00}, // 9
    {0x00,0x36,0x36,0x00,0x00,0x00}, // :
    {0x00,0x56,0x36,0x00,0x00,0x00}, // ;
    {0x08,0x14,0x22,0x41,0x00,0x00}, // <
    {0x14,0x14,0x14,0x14,0x14,0x00}, // =
    {0x00,0x41,0x22,0x14,0x08,0x00}, // >
    {0x02,0x01,0x51,0x09,0x06,0x00}, // ?
    {0x32,0x49,0x79,0x41,0x3E,0x00}, // @
    {0x7E,0x11,0x11,0x11,0x7E,0x00}, // A
    {0x7F,0x49,0x49,0x49,0x36,0x00}, // B
    {0x3E,0x41,0x41,0x41,0x22,0x00}, // C
    {0x7F,0x41,0x41,0x22,0x1C,0x00}, // D
    {0x7F,0x49,0x49,0x49,0x41,0x00}, // E
    {0x7F,0x09,0x09,0x09,0x01,0x00}, // F
    {0x3E,0x41,0x49,0x49,0x7A,0x00}, // G
    {0x7F,0x08,0x08,0x08,0x7F,0x00}, // H
    {0x00,0x41,0x7F,0x41,0x00,0x00}, // I
    {0x20,0x40,0x41,0x3F,0x01,0x00}, // J
    {0x7F,0x08,0x14,0x22,0x41,0x00}, // K
    {0x7F,0x40,0x40,0x40,0x40,0x00}, // L
    {0x7F,0x02,0x0C,0x02,0x7F,0x00}, // M
    {0x7F,0x04,0x08,0x10,0x7F,0x00}, // N
    {0x3E,0x41,0x41,0x41,0x3E,0x00}, // O
    {0x7F,0x09,0x09,0x09,0x06,0x00}, // P
    {0x3E,0x41,0x51,0x21,0x5E,0x00}, // Q
    {0x7F,0x09,0x19,0x29,0x46,0x00}, // R
    {0x46,0x49,0x49,0x49,0x31,0x00}, // S
    {0x01,0x01,0x7F,0x01,0x01,0x00}, // T
    {0x3F,0x40,0x40,0x40,0x3F,0x00}, // U
    {0x1F,0x20,0x40,0x20,0x1F,0x00}, // V
    {0x3F,0x40,0x38,0x40,0x3F,0x00}, // W
    {0x63,0x14,0x08,0x14,0x63,0x00}, // X
    {0x07,0x08,0x70,0x08,0x07,0x00}, // Y
    {0x61,0x51,0x49,0x45,0x43,0x00}, // Z
    {0x00,0x7F,0x41,0x41,0x00,0x00}, // [
    {0x02,0x04,0x08,0x10,0x20,0x00}, // "\"
    {0x00,0x41,0x41,0x7F,0x00,0x00}, // ]
    {0x04,0x02,0x01,0x02,0x04,0x00}, // ^
    {0x40,0x40,0x40,0x40,0x40,0x00}, // _
    {0x00,0x01,0x02,0x04,0x00,0x00}, // `
    {0x20,0x54,0x54,0x54,0x78,0x00}, // a
    {0x7F,0x48,0x44,0x44,0x38,0x00}, // b
    {0x38,0x44,0x44,0x44,0x20,0x00}, // c
    {0x38,0x44,0x44,0x48,0x7F,0x00}, // d
    {0x38,0x54,0x54,0x54,0x18,0x00}, // e
    {0x08,0x7E,0x09,0x01,0x02,0x00}, // f
    {0x0C,0x52,0x52,0x52,0x3E,0x00}, // g
    {0x7F,0x08,0x04,0x04,0x78,0x00}, // h
    {0x00,0x44,0x7D,0x40,0x00,0x00}, // i
    {0x20,0x40,0x44,0x3D,0x00,0x00}, // j
    {0x7F,0x10,0x28,0x44,0x00,0x00}, // k
    {0x00,0x41,0x7F,0x40,0x00,0x00}, // l
    {0x7C,0x04,0x18,0x04,0x78,0x00}, // m
    {0x7C,0x08,0x04,0x04,0x78,0x00}, // n
    {0x38,0x44,0x44,0x44,0x38,0x00}, // o
    {0x7C,0x14,0x14,0x14,0x08,0x00}, // p
    {0x08,0x14,0x14,0x18,0x7C,0x00}, // q
    {0x7C,0x08,0x04,0x04,0x08,0x00}, // r
    {0x48,0x54,0x54,0x54,0x20,0x00}, // s
    {0x04,0x3F,0x44,0x40,0x20,0x00}, // t
    {0x3C,0x40,0x40,0x20,0x7C,0x00}, // u
    {0x1C,0x20,0x40,0x20,0x1C,0x00}, // v
    {0x3C,0x40,0x30,0x40,0x3C,0x00}, // w
    {0x44,0x28,0x10,0x28,0x44,0x00}, // x
    {0x0C,0x50,0x50,0x50,0x3C,0x00}, // y
    {0x44,0x64,0x54,0x4C,0x44,0x00}, // z
    {0x00,0x08,0x36,0x41,0x00,0x00}, // {
    {0x00,0x00,0x7F,0x00,0x00,0x00}, // |
    {0x00,0x41,0x36,0x08,0x00,0x00}, // }
    {0x10,0x08,0x08,0x10,0x08,0x00}, // ~
};

// 向OLED写入一个字节
void OLED_Write_Byte(uint8_t dat, uint8_t cmd)
{
    uint8_t data[2];
    
    if(cmd == OLED_CMD)  // 注意：根据你的oled.h，OLED_CMD=0，OLED_DATA=1
    {
        data[0] = 0x00;  // 命令
    }
    else
    {
        data[0] = 0x40;  // 数据
    }
    
    data[1] = dat;
    
    // 使用HAL_I2C_Master_Transmit发送数据
    HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDRESS, data, 2, HAL_MAX_DELAY);
}

// OLED初始化（改进版本）
void OLED_Init(void)
{
    HAL_Delay(200);  // 等待OLED上电稳定
    
    // 检查I2C设备是否就绪
    if(HAL_I2C_IsDeviceReady(&hi2c1, OLED_ADDRESS, 5, 10) != HAL_OK)
    {
        // 设备未响应，可以在这里添加错误处理
        return;
    }
    
    // 改进的初始化序列
    OLED_Write_Byte(0xAE, OLED_CMD);  // 关闭显示
    
    // 设置时钟分频和振荡器频率
    OLED_Write_Byte(0xD5, OLED_CMD);
    OLED_Write_Byte(0x80, OLED_CMD);
    
    // 设置多路复用率
    OLED_Write_Byte(0xA8, OLED_CMD);
    OLED_Write_Byte(0x3F, OLED_CMD);  // 1/64 duty
    
    // 设置显示偏移
    OLED_Write_Byte(0xD3, OLED_CMD);
    OLED_Write_Byte(0x00, OLED_CMD);  // 无偏移
    
    // 设置起始行
    OLED_Write_Byte(0x40, OLED_CMD);
    
    // 段重映射设置
    OLED_Write_Byte(0xA1, OLED_CMD);  // 段重映射正常
    
    // COM扫描方向
    OLED_Write_Byte(0xC8, OLED_CMD);  // COM扫描方向正常
    
    // COM引脚硬件配置
    OLED_Write_Byte(0xDA, OLED_CMD);
    OLED_Write_Byte(0x12, OLED_CMD);
    
    // 设置对比度（关键参数，影响亮度）
    OLED_Write_Byte(0x81, OLED_CMD);
    OLED_Write_Byte(0xCF, OLED_CMD);  // 对比度207
    
    // 设置预充电周期（关键参数）
    OLED_Write_Byte(0xD9, OLED_CMD);
    OLED_Write_Byte(0xF1, OLED_CMD);
    
    // 设置VCOMH电平（关键参数，影响亮度）
    OLED_Write_Byte(0xDB, OLED_CMD);
    OLED_Write_Byte(0x30, OLED_CMD);  // 0.83xVcc
    
    // 设置内存地址模式
    OLED_Write_Byte(0x20, OLED_CMD);
    OLED_Write_Byte(0x00, OLED_CMD);  // 水平地址模式
    
    // 关闭滚动
    OLED_Write_Byte(0x2E, OLED_CMD);
    
    // 全屏输出跟随RAM
    OLED_Write_Byte(0xA4, OLED_CMD);
    
    // 正常显示（非反色）
    OLED_Write_Byte(0xA6, OLED_CMD);
    
    // 设置电荷泵（关键）
    OLED_Write_Byte(0x8D, OLED_CMD);
    OLED_Write_Byte(0x14, OLED_CMD);  // 启用电荷泵
    
    // 开启显示
    OLED_Write_Byte(0xAF, OLED_CMD);
    
    OLED_Clear();  // 清屏
    HAL_Delay(100);
}

// 清屏
void OLED_Clear(void)
{
    uint8_t i, j;
    
    for(j = 0; j < 8; j++)
    {
        // 设置页地址
        OLED_Write_Byte(0xB0 + j, OLED_CMD);
        // 设置列地址低位
        OLED_Write_Byte(0x00, OLED_CMD);
        // 设置列地址高位
        OLED_Write_Byte(0x10, OLED_CMD);
        
        for(i = 0; i < 128; i++)
        {
            OLED_Write_Byte(0x00, OLED_DATA);
            OLED_GRAM[i][j] = 0x00;
        }
    }
}
// 清除矩形区域
void OLED_Clear_Rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    if(x >= 128 || y >= 8) return;
    
    uint8_t page_start = y;
    uint8_t page_end = y + (height + 7) / 8;
    if(page_end > 8) page_end = 8;
    
    for(uint8_t page = page_start; page < page_end; page++)
    {
        for(uint8_t col = x; col < x + width && col < 128; col++)
        {
            OLED_GRAM[col][page] = 0x00;
        }
    }
    
    // 刷新该区域
    OLED_Refresh_Partial(x, y, width, height);
}

// 部分区域刷新（减少I2C通信量）
void OLED_Refresh_Partial(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    if(x >= 128 || y >= 8) return;
    
    uint8_t page_start = y;
    uint8_t page_end = y + (height + 7) / 8;
    if(page_end > 8) page_end = 8;
    
    for(uint8_t page = page_start; page < page_end; page++)
    {
        OLED_Set_Pos(x, page);
        
        uint8_t col_end = x + width;
        if(col_end > 128) col_end = 128;
        
        for(uint8_t col = x; col < col_end; col++)
        {
            OLED_Write_Byte(OLED_GRAM[col][page], OLED_DATA);
        }
    }
}

// 设置光标位置
void OLED_Set_Pos(uint8_t x, uint8_t y)
{
    if(x >= 128) x = 127;
    if(y >= 8) y = 7;
    
    OLED_Write_Byte(0xB0 + y, OLED_CMD);  // 设置页地址
    OLED_Write_Byte(((x & 0xF0) >> 4) | 0x10, OLED_CMD);  // 设置列地址高位
    OLED_Write_Byte(x & 0x0F, OLED_CMD);  // 设置列地址低位
}

// 显示一个字符
void OLED_Show_Char(uint8_t x, uint8_t y, uint8_t chr, uint8_t size)
{
    uint8_t c = 0, i = 0;
    
    if(size != 6) return;  // 只支持6x8字体
    
    if(chr < ' ') return;  // 过滤不可显示字符
    if(chr > '~') return;  // 过滤不可显示字符
    
    c = chr - ' ';  // 计算偏移
    
    if(x > 128-6)  // 换行
    {
        x = 0;
        y++;
        if(y >= 8) y = 7;  // 防止越界
    }
    
    OLED_Set_Pos(x, y);
    
    for(i = 0; i < 6; i++)
    {
        uint8_t data = ASCII_6x8[c][i];
        OLED_Write_Byte(data, OLED_DATA);
        if(x+i < 128)  // 防止GRAM数组越界
            OLED_GRAM[x+i][y] = data;
    }
}

// 显示字符串
void OLED_Show_String(uint8_t x, uint8_t y, const char *str, uint8_t size)
{
    uint8_t x0 = x;
    
    if(str == NULL) return;
    
    while(*str != '\0')
    {
        if(x0 > 122)  // 换行
        {
            x0 = x;
            y++;
            if(y >= 8) break;  // 超出屏幕底部
        }
        
        OLED_Show_Char(x0, y, *str, size);
        x0 += 6;  // 字符宽度
        str++;
    }
}

// 显示数字
void OLED_Show_Num(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size)
{
    char str[20];
    if(len > 10) len = 10;  // 限制最大长度
    
    // 格式化数字
    sprintf(str, "%*lu", len, num);
    OLED_Show_String(x, y, str, size);
}

// 显示带符号数字
void OLED_Show_SignedNum(uint8_t x, uint8_t y, int32_t num, uint8_t len, uint8_t size)
{
    char str[20];
    if(len > 10) len = 10;  // 限制最大长度
    
    if(num >= 0)
    {
        sprintf(str, "+%*ld", len, (long)num);
    }
    else
    {
        sprintf(str, "-%*ld", len, (long)(-num));
    }
    OLED_Show_String(x, y, str, size);
}

// 显示十六进制数字
void OLED_Show_HexNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size)
{
    char str[20];
    if(len > 8) len = 8;  // 限制最大长度
    
    // 格式化十六进制
    sprintf(str, "%0*lX", len, (unsigned int)num);
    OLED_Show_String(x, y, str, size);
}

// 开显示
void OLED_Display_On(void)
{
    OLED_Write_Byte(0xAF, OLED_CMD);
}

// 关显示
void OLED_Display_Off(void)
{
    OLED_Write_Byte(0xAE, OLED_CMD);
}

// 刷新显示（从GRAM刷新）
void OLED_Refresh(void)
{
    uint8_t i, j;
    
    for(j = 0; j < 8; j++)
    {
        OLED_Write_Byte(0xB0 + j, OLED_CMD);
        OLED_Write_Byte(0x00, OLED_CMD);
        OLED_Write_Byte(0x10, OLED_CMD);
        
        for(i = 0; i < 128; i++)
        {
            OLED_Write_Byte(OLED_GRAM[i][j], OLED_DATA);
        }
    }
}

// 画点
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t)
{
    uint8_t page, bit;
    
    if(x >= 128 || y >= 64) return;
    
    page = y / 8;
    bit = y % 8;
    
    if(t)
    {
        OLED_GRAM[x][page] |= (1 << bit);
    }
    else
    {
        OLED_GRAM[x][page] &= ~(1 << bit);
    }
    
    OLED_Set_Pos(x, page);
    OLED_Write_Byte(OLED_GRAM[x][page], OLED_DATA);
}

// 画线（Bresenham算法）
void OLED_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t mode)
{
    int dx, dy, sx, sy, err, e2;
    
    if(x1 >= 128) x1 = 127;
    if(x2 >= 128) x2 = 127;
    if(y1 >= 64) y1 = 63;
    if(y2 >= 64) y2 = 63;
    
    dx = abs(x2 - x1);
    dy = abs(y2 - y1);
    
    sx = (x1 < x2) ? 1 : -1;
    sy = (y1 < y2) ? 1 : -1;
    
    err = dx - dy;
    
    while(1)
    {
        OLED_DrawPoint(x1, y1, mode);
        
        if(x1 == x2 && y1 == y2) break;
        
        e2 = 2 * err;
        
        if(e2 > -dy)
        {
            err -= dy;
            x1 += sx;
        }
        
        if(e2 < dx)
        {
            err += dx;
            y1 += sy;
        }
    }
}

// 显示中文字符（框架函数，需要字库支持）
void OLED_Show_Chinese(uint8_t x, uint8_t y, uint8_t no, uint8_t size)
{
    // 这里需要中文字库，根据实际情况实现
    // 临时用字母代替
    OLED_Show_Char(x, y, 'C', size);
    OLED_Show_Char(x+6, y, 'H', size);
}


void Show_Bluetooth_Mode(void)
{
    OLED_Clear_Rect(0, 0, 128, 64);
    OLED_Show_String_Safe(0, 0, "蓝牙控制模式", 6);
    OLED_Show_String_Safe(0, 2, "F:前进 B:后退", 6);
    OLED_Show_String_Safe(0, 4, "S:停止 R:康复", 6);
    OLED_Show_String_Safe(0, 6, "M:模式切换", 6);
}

void Show_Voice_Mode(void)
{
    OLED_Clear_Rect(0, 0, 128, 64);
    OLED_Show_String_Safe(0, 0, "语音控制模式", 6);
    OLED_Show_String_Safe(0, 2, "等待语音指令", 6);
    OLED_Show_String_Safe(0, 4, "前进/后退/停止", 6);
    OLED_Show_String_Safe(0, 6, "开始康复训练", 6);
}

void Show_Default_Mode(void)
{
    OLED_Clear();
    
    // 第1行：系统状态
    OLED_Show_String(0, 0, "SYSTEM READY", 6);
    
    // 第2行：当前位置和百分比
    char pos_str[16];
    float current_mm = Motor_Get_Position_MM();
    float current_percent = Motor_Get_Position_Percent();
    sprintf(pos_str, "%5.1fmm (%2.0f%%)", current_mm, current_percent);
    OLED_Show_String(0, 1, "Pos:", 6);
    OLED_Show_String(32, 1, pos_str, 6);
    
    // 第3行：限位信息
    char limit_str[16];
    sprintf(limit_str, "Lim:%.0f-%.0f", g_motor_status.min_limit_mm, g_motor_status.max_limit_mm);
    OLED_Show_String(0, 2, limit_str, 6);
    
    // 第4行：电机状态
    char motor_str[16];
// 修改这里的调用：
    uint8_t dir = Motor_Get_Direction();  // Motor_Get_Direction返回uint8_t
    uint8_t speed = Motor_Get_Speed();
    
    switch(dir) {
        case 1:  // MOTOR_FORWARD
            sprintf(motor_str, "FWD %3d%%", speed);
            break;
        case 2:  // MOTOR_BACKWARD
            sprintf(motor_str, "REV %3d%%", speed);
            break;
        case 0:  // MOTOR_STOP
            sprintf(motor_str, "STOP");
            break;
        case 3:  // MOTOR_BRAKE
            sprintf(motor_str, "BRAKE");
            break;
        default:
            sprintf(motor_str, "UNKNOWN");
            break;
    }
    OLED_Show_String(0, 3, motor_str, 6);
    
    // 第5行：紧急停止状态
    if (g_motor_status.emergency_stop_flag) {
        OLED_Show_String(0, 4, "EMERGENCY STOP!", 6);
    } else {
        OLED_Show_String(0, 4, "System Normal", 6);
    }
    
    // 第6-7行：命令提示
    OLED_Show_String(0, 5, "1:FWD 2:STOP 3:REV", 6);
    OLED_Show_String(0, 6, "X:EMG R:Rehab H:Help", 6);
    
    OLED_Refresh();
}
// OLED显示管理函数（为了兼容原有代码）
void OLED_Force_Refresh(void)
{
    OLED_Refresh();
}

void OLED_Set_Page(uint8_t page)
{
    // 简化：只设置当前页面
    // 在实际应用中，这里可以切换不同的显示页面
    switch(page) {
        case 0:  // PAGE_MAIN_STATUS
            Show_Default_Mode();
            break;
        case 1:  // PAGE_MOTOR_INFO
            OLED_Clear();
            OLED_Show_String(0, 0, "Motor Info", 6);
            OLED_Refresh();
            break;
        case 2:  // PAGE_REHAB_MODE
            Show_Rehabilitation_Mode();
            break;
        case 3:  // PAGE_ERROR
            OLED_Clear();
            OLED_Show_String(0, 0, "ERROR PAGE", 6);
            OLED_Refresh();
            break;
    }
}

void OLED_Show_Error(const char* error_msg)
{
    OLED_Clear();
    if (error_msg != NULL) {
        OLED_Show_String(0, 0, "ERROR:", 6);
        OLED_Show_String(0, 2, (char*)error_msg, 6);
    } else {
        OLED_Show_String(0, 0, "ERROR", 6);
        OLED_Show_String(0, 2, "Unknown Error", 6);
    }
    OLED_Refresh();
}

void OLED_Update_Display(void)
{
    // 定期更新显示
    // 这里可以调用 OLED_Refresh() 或者部分刷新
    OLED_Refresh();
}

// OLED清除函数（替代原来的OLED_Clear）
void OLED_Clear_Display(void)
{
    OLED_Clear();
}

// OLED设置亮度
void OLED_Set_Brightness(uint8_t brightness)
{
    // 设置对比度命令
    OLED_Write_Byte(0x81, OLED_CMD);  // 设置对比度命令
    OLED_Write_Byte(brightness, OLED_CMD);  // 对比度值
}

//// 初始化OLED显示（兼容原有接口）
//void OLED_Display_Init(I2C_HandleTypeDef* i2c)
//{
//    (void)i2c;  // 暂时忽略参数，使用全局hi2c1
//    OLED_Init();
//    OLED_Clear();
//}

//// 简化的显示主状态函数
//void OLED_Show_Main_Status(void)
//{
//    Show_Default_Mode();
//}

// 显示电机信息
void OLED_Show_Motor_Info(void)
{
    OLED_Clear();
    OLED_Show_String(0, 0, "Motor Info", 6);
    
    // 这里可以添加电机具体信息
    char info[32];
    // 假设有全局电机状态变量
    // sprintf(info, "Speed: %d%%", motor_speed);
    // OLED_Show_String(0, 2, info, 6);
    
    OLED_Refresh();
}

// 显示康复模式信息
void Show_Rehabilitation_Mode(void)
{
    OLED_Clear();
    
    // 第一行：康复模式标题
    OLED_Show_String(0, 0, "REHAB MODE", 6);
    
    // 第二行：阻力等级
    char level_str[16];
    uint8_t rehab_level = Rehabilitation_Get_Level();
    sprintf(level_str, "Level: %d/9", rehab_level);
    OLED_Show_String(0, 1, level_str, 6);
    
    // 第三行：是否激活
    char active_str[16];
    uint8_t is_active = Rehabilitation_Is_Active();
    sprintf(active_str, "Active: %s", is_active ? "YES" : "NO");
    OLED_Show_String(0, 2, active_str, 6);
    
    // 第四行：当前位置（mm）
    char pos_str[16];
    float current_mm = Encoder_Get_Position_MM();
    sprintf(pos_str, "Pos: %.1fmm", current_mm);
    OLED_Show_String(0, 3, pos_str, 6);
    
    // 第五行：当前速度
    char speed_str[16];
    float current_speed = Encoder_Get_Velocity_MM_S();
    sprintf(speed_str, "Speed: %.1fmm/s", current_speed);
    OLED_Show_String(0, 4, speed_str, 6);
    
    // 第六行：电机状态
    char motor_str[16];
    MotorDirection_t dir = Motor_Get_Direction();
    uint8_t speed_percent = Motor_Get_Speed();
    
    switch(dir) {
        case MOTOR_FORWARD:
            sprintf(motor_str, "Motor: FWD %d%%", speed_percent);
            break;
        case MOTOR_BACKWARD:
            sprintf(motor_str, "Motor: REV %d%%", speed_percent);
            break;
        default:
            sprintf(motor_str, "Motor: STOP");
            break;
    }
    OLED_Show_String(0, 5, motor_str, 6);
    
    // 第七行：提示信息
    OLED_Show_String(0, 6, "M:Stop U/D:Res +/-", 6);
    
    OLED_Refresh();
}

// 显示康复模式
void OLED_Show_Rehab_Mode(void)
{
    Show_Rehabilitation_Mode();
}

// 测试函数：显示全屏图案
// 填充屏幕
void OLED_Fill(uint8_t fill_Data)
{
    uint8_t i, j;
    
    for(j = 0; j < 8; j++)
    {
        OLED_Write_Byte(0xB0 + j, OLED_CMD);
        OLED_Write_Byte(0x00, OLED_CMD);
        OLED_Write_Byte(0x10, OLED_CMD);
        
        for(i = 0; i < 128; i++)
        {
            OLED_Write_Byte(fill_Data, OLED_DATA);
            OLED_GRAM[i][j] = fill_Data;
        }
    }
}


// 安全版本OLED函数（带超时保护）
void OLED_Init_Safe(void)
{
    uint32_t start_time = HAL_GetTick();
    
    // 调用原始初始化函数
    OLED_Init();
    
    // 检查耗时
    if(HAL_GetTick() - start_time > 500)
    {
        // 初始化超时，可能有问题
    }
}

void OLED_Clear_Safe(void)
{
    uint32_t start_time = HAL_GetTick();
    
    OLED_Clear();
    
    // 设置超时（50ms）
    if(HAL_GetTick() - start_time > 50)
    {
        // 清屏超时
    }
}

void OLED_Show_String_Safe(uint8_t x, uint8_t y, const char *str, uint8_t size)
{
    uint32_t start_time = HAL_GetTick();
    
    // 检查字符串是否有效
    if(str == NULL) return;
    
    // 检查长度限制
    if(strlen(str) > 30) return;  // 防止过长字符串
    
    OLED_Show_String(x, y, str, size);
    
    // 设置超时（每个字符1ms）
    uint32_t max_time = strlen(str) * 1 + 10; // 10ms额外时间
    if(HAL_GetTick() - start_time > max_time)
    {
        // 显示超时
        return;
    }
}

void OLED_Refresh_Safe(void)
{
    uint32_t start_time = HAL_GetTick();
    
    OLED_Refresh();
    
    // 设置超时（100ms）
    if(HAL_GetTick() - start_time > 100)
    {
        // 刷新超时
    }
}

// 简单测试：点亮屏幕
void OLED_Simple_Test(void)
{
    // 全屏点亮测试
    OLED_Fill(0xFF);  // 全亮
    HAL_Delay(1000);
    
    OLED_Clear();     // 清屏
    HAL_Delay(500);
    
    // 显示测试文字
    OLED_Show_String(20, 0, "OLED Display", 6);
    OLED_Show_String(10, 2, "Hardware I2C", 6);
    OLED_Show_String(40, 4, "Test", 6);
    OLED_Show_String(10, 6, "Success!", 6);
    
    OLED_Refresh();
}
