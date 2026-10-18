#include "voice_handler.h"
#include "motor_controller.h"
#include "encoder_reader.h"
#include "rehabilitation_mode.h"
#include <string.h>
#include <stdio.h>

VoiceState_t g_voice_state = {
    .huart = NULL,
    .rx_index = 0,
    .connected = 0,
    .last_activity = 0,
    .new_command = 0
};

static uint8_t s_current_speed = 30;

void Voice_Init(UART_HandleTypeDef* huart)
{
    g_voice_state.huart = huart;
    g_voice_state.rx_index = 0;
    g_voice_state.connected = 1;
    g_voice_state.last_activity = HAL_GetTick();
    g_voice_state.new_command = 0;
    
    memset(g_voice_state.rx_buffer, 0, VOICE_RX_BUFFER_SIZE);
    memset(g_voice_state.last_command, 0, 16);
    
    Voice_Start_Receive();
}

void Voice_Start_Receive(void)
{
    if (g_voice_state.huart == NULL || !g_voice_state.connected) return;
    
    HAL_UART_Receive_IT(g_voice_state.huart, 
                        &g_voice_state.rx_buffer[g_voice_state.rx_index], 
                        1);
}

void Voice_Process_Data(void)
{
    if (!g_voice_state.new_command) return;
    
    g_voice_state.new_command = 0;
    
    Voice_Execute_Command(g_voice_state.last_command);
    
    g_voice_state.rx_index = 0;
    memset(g_voice_state.rx_buffer, 0, VOICE_RX_BUFFER_SIZE);
    memset(g_voice_state.last_command, 0, 16);
}

void Voice_Process_Command(void)
{
    Voice_Process_Data();
}

uint8_t Voice_Has_Command(void)
{
    return g_voice_state.new_command;
}

char* Voice_Get_Last_Command(void)
{
    return g_voice_state.last_command;
}

void Voice_Send_Response(const char* response)
{
    if (g_voice_state.huart == NULL || !g_voice_state.connected) return;
    
    HAL_UART_Transmit(g_voice_state.huart, 
                      (uint8_t*)response, 
                      strlen(response), 
                      100);
}

void Voice_Execute_Command(const char* cmd)
{
    if (strcmp(cmd, VOICE_CMD_FORWARD) == 0) {
        Motor_Go_Forward(s_current_speed);
        Voice_Send_Response("OK:FWD\r\n");
    } 
    else if (strcmp(cmd, VOICE_CMD_BACKWARD) == 0) {
        Motor_Go_Backward(s_current_speed);
        Voice_Send_Response("OK:BWD\r\n");
    } 
    else if (strcmp(cmd, VOICE_CMD_STOP) == 0) {
        Motor_Stop();
        Voice_Send_Response("OK:STP\r\n");
    } 
    else if (strcmp(cmd, VOICE_CMD_SPEED_UP) == 0) {
        s_current_speed += 10;
        if (s_current_speed > 100) s_current_speed = 100;
        char response[32];
        sprintf(response, "OK:SPEED:%d\r\n", s_current_speed);
        Voice_Send_Response(response);
    } 
    else if (strcmp(cmd, VOICE_CMD_SPEED_DOWN) == 0) {
        s_current_speed -= 10;
        if (s_current_speed < 10) s_current_speed = 10;
        char response[32];
        sprintf(response, "OK:SPEED:%d\r\n", s_current_speed);
        Voice_Send_Response(response);
    } 
    else if (strcmp(cmd, VOICE_CMD_REHAB_ON) == 0) {
        Rehabilitation_Mode_Start();
        Voice_Send_Response("OK:RHO\r\n");
    } 
    else if (strcmp(cmd, VOICE_CMD_REHAB_OFF) == 0) {
        Rehabilitation_Mode_Stop();
        Voice_Send_Response("OK:RHF\r\n");
    } 
    else if (strcmp(cmd, VOICE_CMD_RESIST_UP) == 0) {
        uint8_t level = g_rehab_params.resistance_level + 1;
        if (level > 10) level = 10;
        Set_Resistance_Level(level);
        char response[32];
        sprintf(response, "OK:RSL:%d\r\n", level);
        Voice_Send_Response(response);
    } 
    else if (strcmp(cmd, VOICE_CMD_RESIST_DOWN) == 0) {
        uint8_t level = g_rehab_params.resistance_level - 1;
        if (level < 1) level = 1;
        Set_Resistance_Level(level);
        char response[32];
        sprintf(response, "OK:RSL:%d\r\n", level);
        Voice_Send_Response(response);
    } 
    else if (strcmp(cmd, VOICE_CMD_EMERGENCY) == 0) {
        Motor_Emergency_Brake();
        Voice_Send_Response("OK:EMG\r\n");
    }
}

void Voice_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2 && g_voice_state.connected) {
        g_voice_state.last_activity = HAL_GetTick();
        
        // 检查是否是换行符或回车符
        if (g_voice_state.rx_buffer[g_voice_state.rx_index] == '\n' || 
            g_voice_state.rx_buffer[g_voice_state.rx_index] == '\r') {
            
            // 命令结束
            g_voice_state.rx_buffer[g_voice_state.rx_index] = '\0';
            
            // 复制命令
            strncpy(g_voice_state.last_command, 
                   (char*)g_voice_state.rx_buffer, 
                   15);
            g_voice_state.last_command[15] = '\0';
            
            g_voice_state.new_command = 1;
            g_voice_state.rx_index = 0;
        } 
        else {
            // 继续接收
            g_voice_state.rx_index++;
            if (g_voice_state.rx_index >= VOICE_RX_BUFFER_SIZE - 1) {
                g_voice_state.rx_index = 0;
            }
        }
        
        Voice_Start_Receive();
    }
}
