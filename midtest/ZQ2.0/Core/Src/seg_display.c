#include "seg_display.h"

// 共阳极段码表（0-9）
const uint8_t SEG_TABLE[10] = {
    0xC0, // 0
    0xF9, // 1
    0xA4, // 2
    0xB0, // 3
    0x99, // 4
    0x92, // 5
    0x82, // 6
    0xF8, // 7
    0x80, // 8
    0x90  // 9
};

static uint8_t digitBuf[4] = {0};  // 4位数字缓冲区
static uint8_t scanPos = 0;        // 动态扫描位置（0-3）

// 初始化数码管（关闭所有段和位）
void SegDisplay_Init(void) {
    // 关闭所有位选
    HAL_GPIO_WritePin(GPIOF, LED_1_Pin|LED_2_Pin|LED_3_Pin|LED_4_Pin, GPIO_PIN_SET);
    // 关闭所有段选
    HAL_GPIO_WritePin(GPIOC, LED_A_Pin|LED_B_Pin|LED_C_Pin|LED_D_Pin|
                     LED_E_Pin|LED_F_Pin|LED_G_Pin|LED_DP_Pin, GPIO_PIN_SET);
}

// 解析角度值到4位数字（处理前导零）
static void SegDisplay_ParseAngle(uint16_t angle) {
    digitBuf[0] = angle / 1000;    // 千位（0-180，恒为0）
    digitBuf[1] = (angle % 1000) / 100;  // 百位（0-1）
    digitBuf[2] = (angle % 100) / 10;    // 十位
    digitBuf[3] = angle % 10;            // 个位

    // 前导零处理（高位零不显示）
    if (digitBuf[0] == 0) digitBuf[0] = 0xFF;  // 0xFF表示不显示
    if (digitBuf[1] == 0 && digitBuf[0] == 0xFF) digitBuf[1] = 0xFF;
}

// 更新显示角度（调用后更新缓冲区，由定时中断刷新）
void SegDisplay_Update(uint16_t angle) {
    SegDisplay_ParseAngle(angle);
}

// 闪烁n次（每次闪烁：灭200ms->亮200ms）
void SegDisplay_Blink(uint8_t times) {
    for (uint8_t i = 0; i < times; i++) {
        // 熄灭
        HAL_GPIO_WritePin(GPIOC, LED_A_Pin|LED_B_Pin|LED_C_Pin|LED_D_Pin|
                         LED_E_Pin|LED_F_Pin|LED_G_Pin|LED_DP_Pin, GPIO_PIN_SET);
        HAL_Delay(200);
        // 恢复显示
        SegDisplay_Update(inputAngle);
        HAL_Delay(200);
    }
}

// 动态扫描刷新（由TIM3中断调用，1ms一次）
void SegDisplay_Scan(void) {
    // 关闭所有位选
    HAL_GPIO_WritePin(GPIOF, LED_1_Pin|LED_2_Pin|LED_3_Pin|LED_4_Pin, GPIO_PIN_SET);

    // 输出当前位的段码（0xFF表示不显示）
    if (digitBuf[scanPos] != 0xFF) {
        HAL_GPIO_WritePin(GPIOC, LED_A_Pin|LED_B_Pin|LED_C_Pin|LED_D_Pin|
                         LED_E_Pin|LED_F_Pin|LED_G_Pin|LED_DP_Pin, GPIO_PIN_SET);
        GPIOC->BSRR = SEG_TABLE[digitBuf[scanPos]] << 16;  // 段选输出（清除对应位）
    } else {
        // 不显示时关闭所有段
        HAL_GPIO_WritePin(GPIOC, LED_A_Pin|LED_B_Pin|LED_C_Pin|LED_D_Pin|
                         LED_E_Pin|LED_F_Pin|LED_G_Pin|LED_DP_Pin, GPIO_PIN_SET);
    }

    // 打开当前位选
    switch (scanPos) {
        case 0: HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_RESET); break;
        case 1: HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_RESET); break;
        case 2: HAL_GPIO_WritePin(LED_3_GPIO_Port, LED_3_Pin, GPIO_PIN_RESET); break;
        case 3: HAL_GPIO_WritePin(LED_4_GPIO_Port, LED_4_Pin, GPIO_PIN_RESET); break;
    }

    // 切换到下一位
    scanPos = (scanPos + 1) % 4;
}
