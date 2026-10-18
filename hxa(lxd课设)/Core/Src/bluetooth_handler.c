#include "bluetooth_handler.h"
#include "stm32f1xx_hal.h"
#include "motor_controller.h"
#include "encoder_reader.h"
#include "rehabilitation_mode.h"
#include "voice_handler.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

BluetoothState_t g_bluetooth_state = {
    .huart = NULL,
    .rx_index = 0,
    .tx_index = 0,
    .connected = 0,
    .last_activity = 0,
    .cmd_queue_head = 0,
    .cmd_queue_tail = 0
};

// 蓝牙命令执行状态（用于互斥保护）
volatile uint8_t g_bt_command_executing = 0;

void Bluetooth_Init(UART_HandleTypeDef* huart)
{
    DEBUG_PRINT("[BT_INIT] Initializing Bluetooth...\r\n");
    DEBUG_PRINT("[BT_INIT] UART handle: %p\r\n", huart);
    
    g_bluetooth_state.huart = huart;
    g_bluetooth_state.rx_index = 0;
    g_bluetooth_state.connected = 1;
    g_bluetooth_state.last_activity = HAL_GetTick();
    g_bluetooth_state.cmd_queue_head = 0;
    g_bluetooth_state.cmd_queue_tail = 0;
    
    memset(g_bluetooth_state.rx_buffer, 0, BT_RX_BUFFER_SIZE);
    memset(g_bluetooth_state.tx_buffer, 0, BT_TX_BUFFER_SIZE);
    memset(g_bluetooth_state.cmd_queue, 0, BT_CMD_QUEUE_SIZE);
    
    // 启动DMA接收
    DEBUG_PRINT("[BT_INIT] Starting DMA reception...\r\n");
    Bluetooth_Start_Receive();
    DEBUG_PRINT("[BT_INIT] Bluetooth initialized successfully!\r\n");
}

void Bluetooth_Start_Receive(void)
{
    if (g_bluetooth_state.huart == NULL) {
        return;
    }
    
    // 使用中断接收模式
    HAL_UART_Receive_IT(g_bluetooth_state.huart, 
                       &g_bluetooth_state.rx_buffer[g_bluetooth_state.rx_index], 
                       1);
}

void Bluetooth_Send(const uint8_t* data, uint16_t len)
{
    if (g_bluetooth_state.huart && g_bluetooth_state.connected) {
        HAL_UART_Transmit(g_bluetooth_state.huart, data, len, 100);
    }
}

void Bluetooth_Send_String(const char* str)
{
    if (g_bluetooth_state.huart && g_bluetooth_state.connected) {
        HAL_UART_Transmit(g_bluetooth_state.huart, (uint8_t*)str, strlen(str), 100);
    }
}

void Bluetooth_Send_Format(const char* format, ...)
{
    if (!g_bluetooth_state.huart || !g_bluetooth_state.connected) {
        return;
    }
    
    char buffer[BT_TX_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, BT_TX_BUFFER_SIZE, format, args);
    va_end(args);
    
    HAL_UART_Transmit(g_bluetooth_state.huart, (uint8_t*)buffer, strlen(buffer), 100);
}

void Bluetooth_Process_Data(void)
{
    DEBUG_PRINT("[BT] Bluetooth_Process_Data() called, rx_index=%d\r\n", g_bluetooth_state.rx_index);
    
    if (g_bluetooth_state.rx_index == 0) {
        DEBUG_PRINT("[BT] No data to process\r\n");
        return;
    }
    
    DEBUG_PRINT("[BT] Processing %d bytes: ", g_bluetooth_state.rx_index);
    for (uint16_t i = 0; i < g_bluetooth_state.rx_index; i++) {
        DEBUG_PRINT("%c(0x%02X) ", g_bluetooth_state.rx_buffer[i], g_bluetooth_state.rx_buffer[i]);
    }
    DEBUG_PRINT("\r\n");
    
    for (uint16_t i = 0; i < g_bluetooth_state.rx_index; i++) {
        uint8_t cmd = g_bluetooth_state.rx_buffer[i];
        
        if (cmd == '\n' || cmd == '\r') {
            DEBUG_PRINT("[BT] Skipping newline character\r\n");
            continue;
        }
        
        DEBUG_PRINT("[BT] Queueing command: '%c'(0x%02X)\r\n", cmd, cmd);
        g_bluetooth_state.cmd_queue[g_bluetooth_state.cmd_queue_tail] = cmd;
        g_bluetooth_state.cmd_queue_tail = (g_bluetooth_state.cmd_queue_tail + 1) % BT_CMD_QUEUE_SIZE;
    }
    
    g_bluetooth_state.rx_index = 0;
    g_bluetooth_state.last_activity = HAL_GetTick();
    DEBUG_PRINT("[BT] Queue: head=%d, tail=%d\r\n", g_bluetooth_state.cmd_queue_head, g_bluetooth_state.cmd_queue_tail);
}

void Bluetooth_Process_Command(uint8_t cmd)
{
    // 互斥保护：如果有命令正在执行，忽略新命令
    if (g_bt_command_executing) {
        DEBUG_PRINT("[BT] Command ignored - another command is executing\r\n");
        Bluetooth_Send_String("BUSY\r\n");
        return;
    }
    
    g_bt_command_executing = 1;
    DEBUG_PRINT("[BT] Processing command: '%c' (0x%02X)\r\n", cmd, cmd);
    
    switch(cmd) {
        case BT_CMD_STOP:
            DEBUG_PRINT("[BT] >>> CALLING Motor_Stop() <<<\r\n");
            Motor_Stop();
            Bluetooth_Send_String("OK: Stopped\r\n");
            break;
            
        case BT_CMD_FORWARD:
            DEBUG_PRINT("[BT] >>> CALLING Motor_Go_Forward(50) <<<\r\n");
            // 先停止再启动，确保状态干净
            Motor_Stop();
            HAL_Delay(10);
            Motor_Go_Forward(50);
            Bluetooth_Send_String("OK: Forward 50%\r\n");
            break;
            
        case BT_CMD_BACKWARD:
            DEBUG_PRINT("[BT] >>> CALLING Motor_Go_Backward(50) <<<\r\n");
            // 先停止再启动，确保状态干净
            Motor_Stop();
            HAL_Delay(10);
            Motor_Go_Backward(50);
            Bluetooth_Send_String("OK: Backward 50%\r\n");
            break;
            
        case BT_CMD_BRAKE:
            Motor_Brake();
            Bluetooth_Send_String("OK: Brake\r\n");
            break;
            
        case BT_CMD_REHAB_MODE:
            if (g_rehab_params.is_active) {
                Rehabilitation_Mode_Stop();
                Bluetooth_Send_String("OK: Rehab mode stopped\r\n");
            } else {
                Rehabilitation_Mode_Start();
                Bluetooth_Send_String("OK: Rehab mode started\r\n");
            }
            break;
            
        case BT_CMD_REHAB_PLUS:
            if (g_rehab_params.resistance_level < 10) {
                g_rehab_params.resistance_level++;
                Bluetooth_Send_Format("OK: Resistance %d\r\n", g_rehab_params.resistance_level);
            } else {
                Bluetooth_Send_String("ERR: Max resistance\r\n");
            }
            break;
            
        case BT_CMD_REHAB_MINUS:
            if (g_rehab_params.resistance_level > 1) {
                g_rehab_params.resistance_level--;
                Bluetooth_Send_Format("OK: Resistance %d\r\n", g_rehab_params.resistance_level);
            } else {
                Bluetooth_Send_String("ERR: Min resistance\r\n");
            }
            break;
            
        case BT_CMD_EMERGENCY:
            Motor_Emergency_Stop();
            Rehabilitation_Mode_Stop();
            Bluetooth_Send_String("OK: Emergency stop\r\n");
            break;
            
        case BT_CMD_STATUS:
            Bluetooth_Send_Status();
            break;
            
        case BT_CMD_POSITION:
            Bluetooth_Send_Position();
            break;
            
        case BT_CMD_IMPEDANCE:
            if (g_rehab_params.is_active) {
                Impedance_Mode_Enable(1);
                Bluetooth_Send_String("OK: Impedance mode enabled\r\n");
            } else {
                Bluetooth_Send_String("ERR: Rehab mode not active\r\n");
            }
            break;
            
        case BT_CMD_CALIBRATE:
            Bluetooth_Send_String("OK: Calibrating...\r\n");
            break;
            
        case '4':  // Forward 30%
            Motor_Stop();
            HAL_Delay(10);
            Motor_Go_Forward(30);
            Bluetooth_Send_String("OK: Forward 30%\r\n");
            break;
            
        case '5':  // Backward 30%
            Motor_Stop();
            HAL_Delay(10);
            Motor_Go_Backward(30);
            Bluetooth_Send_String("OK: Backward 30%\r\n");
            break;
            
        case '6':  // Forward 70%
            Motor_Stop();
            HAL_Delay(10);
            Motor_Go_Forward(70);
            Bluetooth_Send_String("OK: Forward 70%\r\n");
            break;
            
        case '7':  // Backward 70%
            Motor_Stop();
            HAL_Delay(10);
            Motor_Go_Backward(70);
            Bluetooth_Send_String("OK: Backward 70%\r\n");
            break;
            
        case 'M':  // Move to center (50%)
            Motor_Stop();
            HAL_Delay(10);
            Motor_Move_To_Percent(50, 50);
            Bluetooth_Send_String("OK: Moving to center\r\n");
            break;
            
        case 'Z':  // Set zero position
            Encoder_Set_Zero();
            Bluetooth_Send_String("OK: Zero position set\r\n");
            break;
            
        case 'T':  // Motor test
            Motor_Test_Routine();
            Bluetooth_Send_String("OK: Motor test completed\r\n");
            break;
            
        case 'E':  // Encoder info
            Bluetooth_Send_Format("Encoder: %ld counts, %.1fmm, %.1fmm/s\r\n",
                                 Encoder_Get_Count(),
                                 Encoder_Get_Position_MM(),
                                 Encoder_Get_Velocity_MM_S());
            break;
            
        case 'V':  // Version
            Bluetooth_Send_String("Rehab Arm v1.0\r\n");
            break;
            
        case BT_CMD_HELP:
            Bluetooth_Send_String("=== Help ===\r\n");
            Bluetooth_Send_String("0: Stop\r\n");
            Bluetooth_Send_String("1: Forward 50%\r\n");
            Bluetooth_Send_String("2: Backward 50%\r\n");
            Bluetooth_Send_String("3: Brake\r\n");
            Bluetooth_Send_String("4: Forward 30%\r\n");
            Bluetooth_Send_String("5: Backward 30%\r\n");
            Bluetooth_Send_String("6: Forward 70%\r\n");
            Bluetooth_Send_String("7: Backward 70%\r\n");
            Bluetooth_Send_String("M: Move to center\r\n");
            Bluetooth_Send_String("Z: Set zero position\r\n");
            Bluetooth_Send_String("R: Rehab mode\r\n");
            Bluetooth_Send_String("U: Resistance +\r\n");
            Bluetooth_Send_String("D: Resistance -\r\n");
            Bluetooth_Send_String("X: Emergency stop\r\n");
            Bluetooth_Send_String("S: Status\r\n");
            Bluetooth_Send_String("P: Position\r\n");
            Bluetooth_Send_String("E: Encoder info\r\n");
            Bluetooth_Send_String("I: Impedance mode\r\n");
            Bluetooth_Send_String("C: Calibrate\r\n");
            Bluetooth_Send_String("T: Motor test\r\n");
            Bluetooth_Send_String("V: Version\r\n");
            Bluetooth_Send_String("H: Help\r\n");
            Bluetooth_Send_String("============\r\n");
            break;
            
        default:
            Bluetooth_Send_Format("ERR: Unknown command '%c'\r\n", cmd);
            break;
    }
    
    // 清除命令执行标志
    g_bt_command_executing = 0;
}

uint8_t Bluetooth_Has_Command(void)
{
    return (g_bluetooth_state.cmd_queue_head != g_bluetooth_state.cmd_queue_tail);
}

uint8_t Bluetooth_Get_Command(void)
{
    if (!Bluetooth_Has_Command()) {
        return 0;
    }
    
    uint8_t cmd = g_bluetooth_state.cmd_queue[g_bluetooth_state.cmd_queue_head];
    g_bluetooth_state.cmd_queue_head = (g_bluetooth_state.cmd_queue_head + 1) % BT_CMD_QUEUE_SIZE;
    
    return cmd;
}

void Bluetooth_Send_Status(void)
{
    char buffer[BT_TX_BUFFER_SIZE];
    
    const char* mode_str = "Manual";
    if (Motor_Is_Emergency_Stop()) {
        mode_str = "Emergency";
    } else if (g_rehab_params.is_active) {
        mode_str = "Rehab";
    }
    
    const char* dir_str = "Stop";
    MotorDirection_t dir = Motor_Get_Direction();
    if (dir == MOTOR_FORWARD) dir_str = "Forward";
    else if (dir == MOTOR_BACKWARD) dir_str = "Backward";
    
    snprintf(buffer, BT_TX_BUFFER_SIZE, 
             "=== Status ===\r\n"
             "Mode: %s\r\n"
             "Direction: %s\r\n"
             "Speed: %d%%\r\n"
             "Rehab: %s\r\n"
             "Resistance: %d\r\n"
             "Position: %.1fmm (%.0f%%)\r\n"
             "Velocity: %.1fmm/s\r\n"
             "Encoder: %ld counts\r\n"
             "============\r\n",
             mode_str,
             dir_str,
             Motor_Get_Speed(),
             (g_rehab_params.is_active ? "ON" : "OFF"),
             g_rehab_params.resistance_level,
             Motor_Get_Position_MM(),
             Motor_Get_Position_Percent(),
             Encoder_Get_Velocity_MM_S(),
             Encoder_Get_Count());
    
    Bluetooth_Send_String(buffer);
}

void Bluetooth_Send_Position(void)
{
    char buffer[BT_TX_BUFFER_SIZE];
    
    snprintf(buffer, BT_TX_BUFFER_SIZE,
             "Position: %.1fmm (%.0f%%)\r\n"
             "Velocity: %.1fmm/s\r\n"
             "Encoder: %ld counts\r\n",
             Motor_Get_Position_MM(),
             Motor_Get_Position_Percent(),
             Encoder_Get_Velocity_MM_S(),
             Encoder_Get_Count());
    
    Bluetooth_Send_String(buffer);
}

void Bluetooth_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        // 快速处理，减少调试信息输出
        uint8_t received_char = g_bluetooth_state.rx_buffer[g_bluetooth_state.rx_index];
        
        // 如果是命令字符，直接处理，不等待缓冲区
        if ((received_char >= '0' && received_char <= '9') ||
            (received_char >= 'A' && received_char <= 'Z') ||
            received_char == 'X' || received_char == 'R' || received_char == 'U' || 
            received_char == 'D' || received_char == 'S' || received_char == 'P' ||
            received_char == 'E' || received_char == 'I' || received_char == 'C' ||
            received_char == 'T' || received_char == 'V' || received_char == 'H' ||
            received_char == 'M' || received_char == 'Z') {
            
            // 直接处理这个命令
            DEBUG_PRINT("[BT] Received command: '%c'(0x%02X)\r\n", received_char, received_char);
            g_bluetooth_state.cmd_queue[g_bluetooth_state.cmd_queue_tail] = received_char;
            g_bluetooth_state.cmd_queue_tail = (g_bluetooth_state.cmd_queue_tail + 1) % BT_CMD_QUEUE_SIZE;
        } else {
            // 其他字符，存入缓冲区
            if (g_bluetooth_state.rx_index < BT_RX_BUFFER_SIZE - 1) {
                g_bluetooth_state.rx_index++;
            }
        }
        
        Bluetooth_Start_Receive();
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    Bluetooth_UART_RxCpltCallback(huart);
    
    if (huart->Instance == USART2) {
        Voice_UART_RxCpltCallback(huart);
    }
}

uint8_t Bluetooth_Cmd_Available(void)
{
    return Bluetooth_Has_Command();
}

uint8_t Bluetooth_Is_Connected(void)
{
    return g_bluetooth_state.connected;
}

void Bluetooth_Handle_Error(void)
{
    // 处理蓝牙错误
}

void Bluetooth_Process_Rx_Data(void)
{
    Bluetooth_Process_Data();
}
