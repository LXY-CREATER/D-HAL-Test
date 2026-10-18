#include "key.h"
#include "adc.h"
#include "usart.h"
// ADC通道切换：1=按键，2=电位器
static void ADC_SelectChannel(uint32_t channel) {
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

// 按键扫描（ADC读取3*4矩阵，阈值需根据硬件校准）
KeyType Key_Scan(void) {
    ADC_SelectChannel(ADC_CHANNEL_1);  // 切换到按键通道
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint16_t adcValue = HAL_ADC_GetValue(&hadc1);

    // 阈值范围（示例，需实际测量校准）
    if (adcValue > 3800) return KEY_NONE;
    else if (adcValue > 3400) return KEY_1;
    else if (adcValue > 3000) return KEY_2;
    else if (adcValue > 2600) return KEY_3;
    else if (adcValue > 2200) return KEY_4;
    else if (adcValue > 1800) return KEY_5;
    else if (adcValue > 1400) return KEY_6;
    else if (adcValue > 1000) return KEY_7;
    else if (adcValue > 700) return KEY_8;
    else if (adcValue > 500) return KEY_9;
    else if (adcValue > 300) return KEY_CONFIRM;  // 10
    else if (adcValue > 150) return KEY_0;        // 11
    else return KEY_CLEAR;                        // 12
}

void KEYPAD_DebugOutput(KeyType key)
{
    char msg[50];
    
    switch(key) {
        case KEY_1: sprintf(msg, "Key 1 Pressed\r\n"); break;
        case KEY_2: sprintf(msg, "Key 2 Pressed\r\n"); break;
        case KEY_3: sprintf(msg, "Key 3 Pressed\r\n"); break;
        case KEY_4: sprintf(msg, "Key 4 Pressed\r\n"); break;
        case KEY_5: sprintf(msg, "Key 5 Pressed\r\n"); break;
        case KEY_6: sprintf(msg, "Key 6 Pressed\r\n"); break;
        case KEY_7: sprintf(msg, "Key 7 Pressed\r\n"); break;
        case KEY_8: sprintf(msg, "Key 8 Pressed\r\n"); break;
        case KEY_9: sprintf(msg, "Key 9 Pressed\r\n"); break;
        case KEY_CONFIRM: sprintf(msg, "Key 10 Pressed\r\n"); break;
        case KEY_0: sprintf(msg, "Key 11 Pressed\r\n"); break;
        case KEY_CLEAR: sprintf(msg, "Key 12 Pressed\r\n"); break;
        case KEY_NONE: sprintf(msg, "Invalid Key\r\n"); break;
        default: return; // KEY_NONE不输出
    }
    
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
}


// 读取电位器角度（ADC_CHANNEL_2）
uint16_t Potentiometer_GetAngle(void) {
    ADC_SelectChannel(ADC_CHANNEL_2);  // 切换到电位器通道
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint16_t adcValue = HAL_ADC_GetValue(&hadc1);
    return map(adcValue, 0, 4095, 0, 180);  // 0-4095映射到0-180
}
