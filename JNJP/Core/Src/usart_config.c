#include "usart_config.h"
#include "decision_logic.h"
#include "sensor.h"
#include <string.h>
#include <stdlib.h>

static uint8_t rxBuffer[RX_BUFFER_SIZE];
static uint8_t rxIndex = 0;
static uint8_t rxComplete = 0;
static uint8_t rxByte = 0;

static void UsartConfig_ParseCommand(char *cmd);

void UsartConfig_Init(void)
{
    rxIndex = 0;
    rxComplete = 0;
    memset(rxBuffer, 0, RX_BUFFER_SIZE);
    UsartConfig_SendString("USART Init OK\r\n");
    HAL_UART_Receive_IT(&huart2, &rxByte, 1);
    UsartConfig_SendString("USART RX IT Started\r\n");
}

void UsartConfig_Process(void)
{
    if (rxComplete) {
        UsartConfig_ParseCommand((char*)rxBuffer);
        rxIndex = 0;
        rxComplete = 0;
        memset(rxBuffer, 0, RX_BUFFER_SIZE);
        UsartConfig_SendString("JNJP> ");
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        if (rxByte == '\r' || rxByte == '\n') {
            if (rxIndex > 0) {
                rxBuffer[rxIndex] = '\0';
                rxComplete = 1;
                HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 10);
            }
        } else if (rxByte == 0x08 || rxByte == 0x7F) {
            if (rxIndex > 0) {
                rxIndex--;
                HAL_UART_Transmit(&huart2, (uint8_t*)"\b \b", 3, 10);
            }
        } else if (rxIndex < RX_BUFFER_SIZE - 1) {
            if (rxByte >= 0x20 && rxByte <= 0x7E) {
                rxBuffer[rxIndex++] = rxByte;
                HAL_UART_Transmit(&huart2, &rxByte, 1, 10);
            }
        }
        HAL_UART_Receive_IT(&huart2, &rxByte, 1);
    }
}

static void UsartConfig_ParseCommand(char *cmd)
{
    char *token;
    char *end;
    
    while (*cmd == ' ') cmd++;
    
    end = cmd + strlen(cmd) - 1;
    while (end > cmd && (*end == ' ' || *end == '\r' || *end == '\n')) {
        *end = '\0';
        end--;
    }
    
    if (strlen(cmd) == 0) {
        return;
    }
    
    // 简化命令处理
    if (strcmp(cmd, "S") == 0) {
        UsartConfig_SendString("\r\n=== Status ===\r\n");
        UsartConfig_SendString("L1: ");
        switch(g_lane1Status.vehicleType) {
            case VEHICLE_NONE: UsartConfig_SendString("N"); break;
            case VEHICLE_SMALL: UsartConfig_SendString("S"); break;
            case VEHICLE_LARGE: UsartConfig_SendString("L"); break;
        }
        UsartConfig_SendString(" ADC(raw): ");
        UsartConfig_SendNumber(Sensor_GetRawValue(0));
        UsartConfig_SendString(" peak: ");
        UsartConfig_SendNumber(Sensor_GetPeakValue(0));
        UsartConfig_SendString(" diff: ");
        UsartConfig_SendNumber(Sensor_GetRawValue(0) > Sensor_GetBaseline(0) ? 
                               Sensor_GetRawValue(0) - Sensor_GetBaseline(0) : 0);
        UsartConfig_SendString(" - ");
        UsartConfig_SendNumber(g_decisionResult.safeSpeed1);
        UsartConfig_SendString(" km/h\r\n");
        
        UsartConfig_SendString("L2: ");
        switch(g_lane2Status.vehicleType) {
            case VEHICLE_NONE: UsartConfig_SendString("N"); break;
            case VEHICLE_SMALL: UsartConfig_SendString("S"); break;
            case VEHICLE_LARGE: UsartConfig_SendString("L"); break;
        }
        UsartConfig_SendString(" ADC(raw): ");
        UsartConfig_SendNumber(Sensor_GetRawValue(1));
        UsartConfig_SendString(" peak: ");
        UsartConfig_SendNumber(Sensor_GetPeakValue(1));
        UsartConfig_SendString(" diff: ");
        UsartConfig_SendNumber(Sensor_GetRawValue(1) > Sensor_GetBaseline(1) ? 
                               Sensor_GetRawValue(1) - Sensor_GetBaseline(1) : 0);
        UsartConfig_SendString(" - ");
        UsartConfig_SendNumber(g_decisionResult.safeSpeed2);
        UsartConfig_SendString(" km/h\r\n");
        
        UsartConfig_SendString("\r\n压电陶瓷说明: 按压后看peak值(保持5秒自动衰减)\r\n");
        UsartConfig_SendString("小龙虾对接: 输入 UPLOAD 输出JSON格式数据\r\n");
    } else if (strcmp(cmd, "UPLOAD") == 0) {
        Sensor_UploadData();
    } else if (strncmp(cmd, "L1 ", 3) == 0) {
        // 车道1命令: L1 N/S/L
        cmd += 3;
        while (*cmd == ' ') cmd++;
        if (strcmp(cmd, "N") == 0) {
            g_lane1Status.pressureValue = 100;
            g_lane1Status.vehicleType = VEHICLE_NONE;
            UsartConfig_SendString("OK: L1=N\r\n");
        } else if (strcmp(cmd, "S") == 0) {
            g_lane1Status.pressureValue = 1500;
            g_lane1Status.vehicleType = VEHICLE_SMALL;
            UsartConfig_SendString("OK: L1=S\r\n");
        } else if (strcmp(cmd, "L") == 0) {
            g_lane1Status.pressureValue = 4000;
            g_lane1Status.vehicleType = VEHICLE_LARGE;
            UsartConfig_SendString("OK: L1=L\r\n");
        }
    } else if (strncmp(cmd, "L2 ", 3) == 0) {
        // 车道2命令: L2 N/S/L
        cmd += 3;
        while (*cmd == ' ') cmd++;
        if (strcmp(cmd, "N") == 0) {
            g_lane2Status.pressureValue = 100;
            g_lane2Status.vehicleType = VEHICLE_NONE;
            UsartConfig_SendString("OK: L2=N\r\n");
        } else if (strcmp(cmd, "S") == 0) {
            g_lane2Status.pressureValue = 1500;
            g_lane2Status.vehicleType = VEHICLE_SMALL;
            UsartConfig_SendString("OK: L2=S\r\n");
        } else if (strcmp(cmd, "L") == 0) {
            g_lane2Status.pressureValue = 4000;
            g_lane2Status.vehicleType = VEHICLE_LARGE;
            UsartConfig_SendString("OK: L2=L\r\n");
        }
    } else if (strcmp(cmd, "C") == 0) {
        // 清除命令: C
        g_lane1Status.vehicleType = VEHICLE_NONE;
        g_lane1Status.pressureValue = 0;
        g_lane1Status.isStable = 0;
        g_lane2Status.vehicleType = VEHICLE_NONE;
        g_lane2Status.pressureValue = 0;
        g_lane2Status.isStable = 0;
        DecisionLogic_Process();
        UsartConfig_SendString("OK: Cleared\r\n");
    } else if (strcmp(cmd, "H") == 0) {
        // 帮助命令: H
        UsartConfig_SendString("\r\n=== Commands ===\r\n");
        UsartConfig_SendString("S - Show status (with ADC values)\r\n");
        UsartConfig_SendString("L1 N/S/L - Set lane 1 vehicle\r\n");
        UsartConfig_SendString("L2 N/S/L - Set lane 2 vehicle\r\n");
        UsartConfig_SendString("C - Clear all lanes\r\n");
        UsartConfig_SendString("CAL - Show calibration guide\r\n");
        UsartConfig_SendString("H - Show this help\r\n");
    } else if (strcmp(cmd, "CAL") == 0) {
        UsartConfig_SendString("\r\n=== Calibration Guide ===\r\n");
        UsartConfig_SendString("Thresholds:\r\n");
        UsartConfig_SendString("  NONE  < ");
        UsartConfig_SendNumber(g_sysConfig.thresholdNone);
        UsartConfig_SendString("\r\n  SMALL  ");
        UsartConfig_SendNumber(g_sysConfig.thresholdNone);
        UsartConfig_SendString(" ~ ");
        UsartConfig_SendNumber(g_sysConfig.thresholdSmall);
        UsartConfig_SendString("\r\n");
        UsartConfig_SendString("  LARGE >= ");
        UsartConfig_SendNumber(g_sysConfig.thresholdSmall);
        UsartConfig_SendString("\r\n\r\n");
        UsartConfig_SendString("Steps:\r\n");
        UsartConfig_SendString("1. No load: ADC should be ~100-300\r\n");
        UsartConfig_SendString("2. Person standing: ~500-1000 (SMALL)\r\n");
        UsartConfig_SendString("3. Car: >1200 (LARGE)\r\n");
    } else {
        UsartConfig_SendString("ERROR: Unknown command. Type H.\r\n");
    }
}

void UsartConfig_SendString(const char *str)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 100);
}

void UsartConfig_SendNumber(int32_t num)
{
    char buf[16];
    int i = 0;
    int neg = 0;
    
    if (num < 0) {
        neg = 1;
        num = -num;
    }
    
    if (num == 0) {
        buf[i++] = '0';
    } else {
        while (num > 0) {
            buf[i++] = '0' + (num % 10);
            num /= 10;
        }
    }
    
    if (neg) {
        buf[i++] = '-';
    }
    
    buf[i] = '\0';
    
    int len = i;
    int j;
    for (j = 0; j < len / 2; j++) {
        char tmp = buf[j];
        buf[j] = buf[len - 1 - j];
        buf[len - 1 - j] = tmp;
    }
    
    HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), 100);
}

void UsartConfig_PrintConfig(void)
{
    UsartConfig_SendString("\r\n=== System Config ===\r\n");
    
    UsartConfig_SendString("Curve Radius: ");
    UsartConfig_SendNumber((int32_t)g_sysConfig.curveRadius);
    UsartConfig_SendString(" m\r\n");
    
    UsartConfig_SendString("Threshold None: ");
    UsartConfig_SendNumber(g_sysConfig.thresholdNone);
    UsartConfig_SendString("\r\n");
    
    UsartConfig_SendString("Threshold Small: ");
    UsartConfig_SendNumber(g_sysConfig.thresholdSmall);
    UsartConfig_SendString("\r\n");
    
    UsartConfig_SendString("Threshold Large: ");
    UsartConfig_SendNumber(g_sysConfig.thresholdLarge);
    UsartConfig_SendString("\r\n");
    
    UsartConfig_SendString("Max Speed Large: ");
    UsartConfig_SendNumber(g_sysConfig.maxSpeedLarge);
    UsartConfig_SendString(" km/h\r\n");
    
    UsartConfig_SendString("Max Speed Small: ");
    UsartConfig_SendNumber(g_sysConfig.maxSpeedSmall);
    UsartConfig_SendString(" km/h\r\n");
}
