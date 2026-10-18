#include "bluetooth.h"
#include "main.h"
//#include "motor_controller.h"
#include "encoder.h"
//#include "rehabilitation.h"
#include <string.h>
#include <stdarg.h>
#include "all_includes.h"
// ============================================
// 全局变量定义
// ============================================

// 1. 在文件开头添加访问函数
static BluetoothState* get_bluetooth_ptr(void) {
    return get_bluetooth_state();
}

static RehabState* get_rehab_ptr(void) {
    return get_rehab_state();
}

BluetoothStatus bt_status = {
    .rx_index = 0,
    .cmd_head = 0,
    .cmd_tail = 0,
    .is_connected = 0,
    .last_rx_time = 0,
    .error_count = 0,
    .dma_active = 0
};

UART_HandleTypeDef* bluetooth_uart = NULL;

// DMA句柄声明（在main.c中定义）
extern DMA_HandleTypeDef hdma_usart1_rx;
// 添加DMA函数的正确定义
#ifndef __HAL_DMA_GET_COUNTER
#define __HAL_DMA_GET_COUNTER(__HANDLE__) ((__HANDLE__)->Instance->CNDTR)
#endif

#ifndef __HAL_DMA_SET_COUNTER
#define __HAL_DMA_SET_COUNTER(__HANDLE__, __COUNTER__) ((__HANDLE__)->Instance->CNDTR = (__COUNTER__))
#endif
// ============================================
// 初始化函数
// ============================================

void Bluetooth_Init(UART_HandleTypeDef* uart)
{
		MotorConfig* config = get_config_ptr();
    MotorState* motor = get_motor_ptr();
	
    bluetooth_uart = uart;
    
    // 清空缓冲区
    memset(bt_status.rx_buffer, 0, sizeof(bt_status.rx_buffer));
    memset(bt_status.tx_buffer, 0, sizeof(bt_status.tx_buffer));
    memset(bt_status.cmd_queue, 0, sizeof(bt_status.cmd_queue));
    
    // 初始化变量
    bt_status.rx_index = 0;
    bt_status.cmd_head = 0;
    bt_status.cmd_tail = 0;
    bt_status.is_connected = 0;
    bt_status.last_rx_time = 0;
    bt_status.error_count = 0;
    bt_status.dma_active = 0;
    
    DEBUG_PRINT("Bluetooth initialized\r\n");
}

void Bluetooth_Deinit(void)
{
    Bluetooth_Stop_Receive();
    bluetooth_uart = NULL;
    bt_status.dma_active = 0;
}

void Bluetooth_Start_Receive(void)
{
    if (!bluetooth_uart || bt_status.dma_active) return;
    
    // 启动DMA接收
    HAL_UART_Receive_DMA(bluetooth_uart, bt_status.rx_buffer, BT_RX_BUFFER_SIZE);
    
    // 启用空闲中断
    __HAL_UART_ENABLE_IT(bluetooth_uart, UART_IT_IDLE);
    
    bt_status.dma_active = 1;
    bt_status.last_rx_time = HAL_GetTick();
    
    DEBUG_PRINT("Bluetooth DMA receive started\r\n");
}

void Bluetooth_Stop_Receive(void)
{
    if (!bluetooth_uart || !bt_status.dma_active) return;
    
    // 禁用空闲中断
    __HAL_UART_DISABLE_IT(bluetooth_uart, UART_IT_IDLE);
    
    // 停止DMA接收
    HAL_UART_DMAStop(bluetooth_uart);
    
    bt_status.dma_active = 0;
    
    DEBUG_PRINT("Bluetooth DMA receive stopped\r\n");
}

// ============================================
// 数据处理函数
// ============================================

void Bluetooth_Process_Rx_Data(void)
{
    static uint8_t processing = 0;
    
    if (processing || !bt_status.dma_active) return;
    processing = 1;
    
    // 检查DMA接收的数据长度
    uint16_t data_length = BT_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
    
    if (data_length > 0 && data_length <= BT_RX_BUFFER_SIZE) {
        // 更新最后接收时间
        bt_status.last_rx_time = HAL_GetTick();
        bt_status.is_connected = 1;
        
        // 解析数据
        Bluetooth_Parse_Data(bt_status.rx_buffer, data_length);
        
        // 重置DMA
        HAL_UART_DMAStop(bluetooth_uart);
        HAL_UART_Receive_DMA(bluetooth_uart, bt_status.rx_buffer, BT_RX_BUFFER_SIZE);
        
        // 重置DMA计数器
        __HAL_DMA_SET_COUNTER(&hdma_usart1_rx, BT_RX_BUFFER_SIZE);
    }
    
    processing = 0;
}

void Bluetooth_Parse_Data(uint8_t* data, uint16_t length)
{
    if (!data || length == 0) return;
    
    for (int i = 0; i < length; i++) {
        uint8_t ch = data[i];
        
        // 跳过控制字符
        if (ch == '\r' || ch == '\n' || ch == ' ' || ch == '\t') {
            continue;
        }
        
        // 转换为大写
        if (ch >= 'a' && ch <= 'z') {
            ch = ch - 'a' + 'A';
        }
        
        // 解析命令
        switch (ch) {
            case 'F':  // 前进
            case 'B':  // 后退
                // 检查是否有速度参数
                if (i + 1 < length) {
                    uint8_t next_ch = data[i + 1];
                    if (next_ch >= '0' && next_ch <= '9') {
                        Bluetooth_Put_Command(ch, next_ch - '0');
                        i++;  // 跳过数字
                    } else {
                        Bluetooth_Put_Command(ch, 5);  // 默认速度5
                    }
                } else {
                    Bluetooth_Put_Command(ch, 5);  // 默认速度5
                }
                break;
                
            case 'S':  // 停止
            case 'R':  // 康复模式
            case 'T':  // 测试
            case 'E':  // 紧急停止
            case 'X':  // 紧急刹车
            case 'C':  // 检查引脚
            case 'M':  // 重置紧急停止
            case 'Z':  // 设置零点
                Bluetooth_Put_Command(ch, 0);
                break;
                
            case '+':  // 加速
            case '-':  // 减速
            case 'U':  // 增加阻力
            case 'D':  // 减少阻力
                Bluetooth_Put_Command(ch, 0);
                break;
                
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                // 数字命令：0-9对应速度0-90%
                Bluetooth_Put_Command('N', ch - '0');
                break;
                
            case 'P':  // 位置命令
                if (i + 1 < length) {
                    uint8_t next_ch = data[i + 1];
                    if (next_ch >= '0' && next_ch <= '9') {
                        Bluetooth_Put_Command(ch, next_ch - '0');
                        i++;  // 跳过数字
                    }
                }
                break;
                
            default:
                // 未知命令
                Bluetooth_Send_Response("Unknown command: %c\r\n", ch);
                break;
        }
    }
}

void Bluetooth_Process_Command(BluetoothCmd cmd)
{
    char response[128];
    
    switch (cmd.command) {
        case 'F':  // 前进
            Motor_Go_Forward(cmd.parameter * 10);
            sprintf(response, "Forward: %d%%\r\n", cmd.parameter * 10);
            break;
            
        case 'B':  // 后退
            Motor_Go_Backward(cmd.parameter * 10);
            sprintf(response, "Backward: %d%%\r\n", cmd.parameter * 10);
            break;
            
        case 'S':  // 停止
            Motor_Stop();
            sprintf(response, "Motor stopped\r\n");
            break;
            
        case 'R':  // 切换康复模式
            if (Rehabilitation_Is_Active()) {
                Rehabilitation_Stop();
                sprintf(response, "Rehab mode stopped\r\n");
            } else {
                Rehabilitation_Start();
                sprintf(response, "Rehab mode started (Level: %d)\r\n", Rehabilitation_Get_Level());
            }
            break;
            
        case 'T':  // 测试
            Motor_Test_Routine();
            sprintf(response, "Test routine executed\r\n");
            break;
            
        case 'E':  // 紧急停止
            Motor_Emergency_Brake();
            sprintf(response, "EMERGENCY STOP!\r\n");
            break;
            
        case 'X':  // 紧急刹车
            Motor_Force_Stop();
            sprintf(response, "FORCE STOP!\r\n");
            break;
            
        case '+':  // 加速
            if (Motor_Get_Speed() < 90) {
                Motor_Set_Speed(Motor_Get_Speed() + 10);
            }
            sprintf(response, "Speed: %d%%\r\n", Motor_Get_Speed());
            break;
            
        case '-':  // 减速
            if (Motor_Get_Speed() > 10) {
                Motor_Set_Speed(Motor_Get_Speed() - 10);
            }
            sprintf(response, "Speed: %d%%\r\n", Motor_Get_Speed());
            break;
            
        case 'U':  // 增加阻力
            if (Rehabilitation_Is_Active()) {
                Rehabilitation_Increase_Level();
                sprintf(response, "Resistance: Level %d\r\n", Rehabilitation_Get_Level());
            } else {
                sprintf(response, "Rehab mode not active\r\n");
            }
            break;
            
        case 'D':  // 减少阻力
            if (Rehabilitation_Is_Active()) {
                Rehabilitation_Decrease_Level();
                sprintf(response, "Resistance: Level %d\r\n", Rehabilitation_Get_Level());
            } else {
                sprintf(response, "Rehab mode not active\r\n");
            }
            break;
            
        case 'N':  // 数字命令
            if (cmd.parameter == 0) {
                Motor_Stop();
                sprintf(response, "Speed: 0%% (stopped)\r\n");
            } else {
                uint8_t speed = cmd.parameter * 10;
                if (Motor_Get_Direction() == MOTOR_FORWARD) {
                    Motor_Go_Forward(speed);
                } else if (Motor_Get_Direction() == MOTOR_BACKWARD) {
                    Motor_Go_Backward(speed);
                } else {
                    Motor_Go_Forward(speed);
                }
                sprintf(response, "Speed: %d%%\r\n", speed);
            }
            break;
            
        case 'P':  // 位置命令
            if (cmd.parameter >= 0 && cmd.parameter <= 9) {
                Motor_Move_To_Percent(cmd.parameter * 10, 30);
                sprintf(response, "Moving to %d%% position\r\n", cmd.parameter * 10);
            }
            break;
            
        case 'C':  // 检查引脚状态
            Motor_Check_Pin_State();
            sprintf(response, "Pin state checked\r\n");
            break;
            
        case 'M':  // 重置紧急停止
            Motor_Reset_Emergency_Stop();
            sprintf(response, "Emergency stop reset\r\n");
            break;
            
        case 'Z':  // 设置零点
            Motor_Set_Zero_Position();
            sprintf(response, "Zero position set\r\n");
            break;
            
        default:
            sprintf(response, "Unknown command: %c\r\n", cmd.command);
            break;
    }
    
    Bluetooth_Send_String(response);
}

// ============================================
// 命令队列管理
// ============================================

void Bluetooth_Put_Command(uint8_t cmd, uint8_t param)
{
    uint8_t next_tail = (bt_status.cmd_tail + 1) % BT_CMD_QUEUE_SIZE;
    
    if (next_tail != bt_status.cmd_head) {
        bt_status.cmd_queue[bt_status.cmd_tail].command = cmd;
        bt_status.cmd_queue[bt_status.cmd_tail].parameter = param;
        bt_status.cmd_queue[bt_status.cmd_tail].timestamp = HAL_GetTick();
        bt_status.cmd_tail = next_tail;
        
        DEBUG_PRINT("BT Cmd queued: %c, param=%d\r\n", cmd, param);
    } else {
        DEBUG_PRINT("BT Cmd queue full!\r\n");
    }
}

uint8_t Bluetooth_Cmd_Available(void)
{
    return (bt_status.cmd_head != bt_status.cmd_tail);
}

BluetoothCmd Bluetooth_Get_Command(void)
{
    BluetoothCmd cmd = {0};
    
    if (Bluetooth_Cmd_Available()) {
        cmd = bt_status.cmd_queue[bt_status.cmd_head];
        bt_status.cmd_head = (bt_status.cmd_head + 1) % BT_CMD_QUEUE_SIZE;
    }
    
    return cmd;
}

void Bluetooth_Clear_Queue(void)
{
    bt_status.cmd_head = bt_status.cmd_tail = 0;
    DEBUG_PRINT("Bluetooth command queue cleared\r\n");
}

void Bluetooth_Clear_Buffer(void)
{
    memset(bt_status.rx_buffer, 0, sizeof(bt_status.rx_buffer));
    memset(bt_status.tx_buffer, 0, sizeof(bt_status.tx_buffer));
    Bluetooth_Clear_Queue();
    
    // 重置DMA
    if (bt_status.dma_active) {
        HAL_UART_DMAStop(bluetooth_uart);
        HAL_UART_Receive_DMA(bluetooth_uart, bt_status.rx_buffer, BT_RX_BUFFER_SIZE);
    }
    
    DEBUG_PRINT("Bluetooth buffers cleared\r\n");
}

// ============================================
// 发送函数
// ============================================

void Bluetooth_Send_Data(uint8_t* data, uint16_t length)
{
    if (!bluetooth_uart || !data || length == 0) return;
    
    HAL_UART_Transmit(bluetooth_uart, data, length, 100);
}

void Bluetooth_Send_String(const char* str)
{
    if (!bluetooth_uart || !str) return;
    
    HAL_UART_Transmit(bluetooth_uart, (uint8_t*)str, strlen(str), 100);
}

void Bluetooth_Send_Response(const char* format, ...)
{
    if (!bluetooth_uart || !format) return;
    
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    Bluetooth_Send_String(buffer);
}

void Bluetooth_Send_Status(void)
{
    char status[256];
    
    sprintf(status,
            "=== System Status ===\r\n"
            "Time: %lums\r\n"
            "Motor: %s, Speed: %d%%\r\n"
            "Position: %.1fmm (%.1f%%)\r\n"
            "Encoder: %ld counts\r\n"
            "Velocity: %.1fmm/s\r\n"
            "Rehab: %s, Level: %d\r\n"
            "Emergency: %s\r\n"
            "Moving: %s\r\n"
            "BT Connected: %s\r\n",
            HAL_GetTick(),
            (Motor_Get_Direction() == MOTOR_FORWARD) ? "FORWARD" :
            (Motor_Get_Direction() == MOTOR_BACKWARD) ? "BACKWARD" :
            (Motor_Get_Direction() == MOTOR_BRAKE) ? "BRAKE" : "STOP",
            Motor_Get_Speed(),
            Motor_Get_Position_MM(),
            Motor_Get_Position_Percent(),
            Encoder_Get_Count(),
            Encoder_Get_Velocity_MM_S(),
            Rehabilitation_Is_Active() ? "ON" : "OFF",
            Rehabilitation_Get_Level(),
            Motor_Is_Emergency_Stop() ? "YES" : "NO",
            Motor_Is_Moving() ? "YES" : "NO",
            Bluetooth_Is_Connected() ? "YES" : "NO");
    
    Bluetooth_Send_String(status);
}

// ============================================
// 状态函数
// ============================================

uint8_t Bluetooth_Is_Connected(void)
{
    // 简单的心跳检测：10秒内收到过数据则认为连接
    if (HAL_GetTick() - bt_status.last_rx_time < 10000) {
        bt_status.is_connected = 1;
    } else {
        bt_status.is_connected = 0;
    }
    
    return bt_status.is_connected;
}

void Bluetooth_Set_Connected(uint8_t connected)
{
    bt_status.is_connected = connected;
    if (connected) {
        bt_status.last_rx_time = HAL_GetTick();
    }
}

uint32_t Bluetooth_Get_Last_Rx_Time(void)
{
    return bt_status.last_rx_time;
}

// ============================================
// 错误处理
// ============================================

void Bluetooth_Handle_Error(void)
{
    bt_status.error_count++;
    
    if (bt_status.error_count > 10) {
        DEBUG_PRINT("Bluetooth error count: %lu, resetting...\r\n", bt_status.error_count);
        Bluetooth_Reset();
    }
}

void Bluetooth_Reset(void)
{
    Bluetooth_Stop_Receive();
    Bluetooth_Clear_Buffer();
    
    bt_status.error_count = 0;
    
    HAL_Delay(100);
    
    Bluetooth_Start_Receive();
    
    DEBUG_PRINT("Bluetooth reset complete\r\n");
}
