#include "voice.h"
#include "all_includes.h"
#include "config.h"
#include <string.h>

// ============================================
// ˽�к���
// ============================================

#include "global_vars.h"

// DMA�������
extern DMA_HandleTypeDef hdma_usart2_rx;

// �ⲿ�������������ļ��������ӣ�
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;

// ��������״̬
static VoiceStatus_t g_voice = {0};

// ȫ�ֱ������壨��app_globals.c����
static SystemStatus_t* get_system_ptr(void) {
    static SystemStatus_t sys = {0};
    return &sys;
}

static MotorStatus_t* get_motor_ptr(void) {
    static MotorStatus_t motor = {0};
    return &motor;
}

// ����DMA��������ȷ����
#ifndef __HAL_DMA_GET_COUNTER
#define __HAL_DMA_GET_COUNTER(__HANDLE__) ((__HANDLE__)->Instance->CNDTR)
#endif

#ifndef __HAL_DMA_SET_COUNTER
#define __HAL_DMA_SET_COUNTER(__HANDLE__, __COUNTER__) ((__HANDLE__)->Instance->CNDTR = (__COUNTER__))
#endif
// ============================================
// ��������ӳ���
// ============================================

// ASRPRO��������ӳ��
typedef struct {
    const char* keyword;      // �ؼ���
    uint8_t length;           // �ؼ��ʳ���
    VoiceCommand_t command;   // ��Ӧ����
} VoiceCommandMap_t;

static const VoiceCommandMap_t VOICE_COMMAND_MAP[] = {
    // ���Ĺؼ��ʣ�ASRPRO�����
    {"QIANJIN", 7, VOICE_CMD_FORWARD},      // ǰ��
    {"HOUTUI", 6, VOICE_CMD_BACKWARD},      // ����
    {"TINGZHI", 7, VOICE_CMD_STOP},         // ֹͣ
    {"FULI", 4, VOICE_CMD_RESISTANCE},      // ����
    {"KANGZU", 6, VOICE_CMD_RESISTANCE},    // ����
    {"JIAKUAI", 7, VOICE_CMD_SPEED_UP},     // �ӿ�
    {"JIAMAN", 6, VOICE_CMD_SPEED_DOWN},    // ����
    {"MOSHI", 5, VOICE_CMD_MODE_CHANGE},    // ģʽ
    {"JIJING", 6, VOICE_CMD_EMERGENCY_STOP},// ��ͣ
    {"CESHI", 5, VOICE_CMD_TEST},           // ����
    
    // Ӣ�Ĺؼ��ʣ����ݣ�
    {"FORWARD", 7, VOICE_CMD_FORWARD},
    {"BACKWARD", 8, VOICE_CMD_BACKWARD},
    {"STOP", 4, VOICE_CMD_STOP},
    {"RESISTANCE", 10, VOICE_CMD_RESISTANCE},
    {"SPEEDUP", 7, VOICE_CMD_SPEED_UP},
    {"SPEEDDOWN", 9, VOICE_CMD_SPEED_DOWN},
    {"MODE", 4, VOICE_CMD_MODE_CHANGE},
    {"EMERGENCY", 9, VOICE_CMD_EMERGENCY_STOP},
    {"TEST", 4, VOICE_CMD_TEST},
    
    // ���ַ��������������
    {"F", 1, VOICE_CMD_FORWARD},
    {"B", 1, VOICE_CMD_BACKWARD},
    {"S", 1, VOICE_CMD_STOP},
    {"R", 1, VOICE_CMD_RESISTANCE},
    {"+", 1, VOICE_CMD_SPEED_UP},
    {"-", 1, VOICE_CMD_SPEED_DOWN},
    {"M", 1, VOICE_CMD_MODE_CHANGE},
    {"E", 1, VOICE_CMD_EMERGENCY_STOP},
    {"T", 1, VOICE_CMD_TEST}
};

// ============================================
// ˽�к�������
// ============================================

static void Voice_Send_Ack(void);
static void Voice_Reset_Buffer(void);
static void Voice_Send_Config(void);

// ============================================
// ��ʼ������
// ============================================

void Voice_Handler_Init(UART_HandleTypeDef* uart)
{
    (void)uart;  // �Ѿ�ͨ��ȫ�ֱ���ʹ��
    
    // ��ջ�����
    memset(g_voice.rx_buffer, 0, sizeof(g_voice.rx_buffer));
    
    // ��ʼ������
    g_voice.last_command = VOICE_CMD_NONE;
    g_voice.last_command_time = 0;
    g_voice.is_ready = 1;
    g_voice.error_count = 0;
    
    DEBUG_PRINT("Voice handler initialized\n");
}

void Voice_Start_Receive(void)
{
    // ����DMA����
    HAL_UART_Receive_DMA(&huart2, g_voice.rx_buffer, sizeof(g_voice.rx_buffer));
    
    // ���ÿ����ж�
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
    
    // ����������������Ҫ��
    Voice_Send_Config();
    
    DEBUG_PRINT("Voice DMA receive started\n");
}

// ============================================
// ���ݴ�������
// ============================================

void Voice_Process_Rx_Data(void)
{
    static uint8_t processing = 0;
    
    if (processing || !g_voice.is_ready) return;
    processing = 1;
    
    // ����Ƿ���������
    uint16_t data_length = sizeof(g_voice.rx_buffer) - __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);
    
    if (data_length > 0 && data_length <= sizeof(g_voice.rx_buffer)) {
        // �����������ʱ��
        g_voice.last_command_time = HAL_GetTick();
        
        // ��������
        VoiceCommand_t cmd = Voice_Parse_Command(g_voice.rx_buffer, data_length);
        
        if (cmd != VOICE_CMD_NONE) {
            // �������һ������
            g_voice.last_command = cmd;
            
            // ִ������
            Voice_Execute_Command(cmd);
            
            // ����ȷ����Ӧ
            Voice_Send_Ack();
        } else {
            // δ֪����
            g_voice.error_count++;
            
            if (get_system_ptr()->debug_enabled) {
                DEBUG_PRINT("Voice: Unknown command\n");
            }
        }
        
        // ���������
        Voice_Reset_Buffer();
        
        // ��������DMA����
        HAL_UART_Receive_DMA(&huart2, g_voice.rx_buffer, sizeof(g_voice.rx_buffer));
    }
    
    processing = 0;
}

VoiceCommand_t Voice_Parse_Command(uint8_t* data, uint8_t length)
{
    if (data == NULL || length == 0) {
        return VOICE_CMD_NONE;
    }
    
    // ������ת��Ϊ�ַ�����ȷ����NULL��β��
    char str_buffer[32] = {0};
    uint8_t copy_len = (length < sizeof(str_buffer) - 1) ? length : sizeof(str_buffer) - 1;
    memcpy(str_buffer, data, copy_len);
    
    // ת��Ϊ��д��ASRPRO���ܷ���Сд��
    for (int i = 0; i < copy_len; i++) {
        if (str_buffer[i] >= 'a' && str_buffer[i] <= 'z') {
            str_buffer[i] = str_buffer[i] - 'a' + 'A';
        }
    }
    
    // �Ƴ����ܵĻ��з��Ϳո�
    for (int i = 0; i < copy_len; i++) {
        if (str_buffer[i] == '\r' || str_buffer[i] == '\n' || 
            str_buffer[i] == ' ' || str_buffer[i] == 0) {
            str_buffer[i] = 0;
            break;
        }
    }
    
    // ����ƥ�������
    for (int i = 0; i < sizeof(VOICE_COMMAND_MAP) / sizeof(VOICE_COMMAND_MAP[0]); i++) {
        if (strncmp(str_buffer, VOICE_COMMAND_MAP[i].keyword, 
                   VOICE_COMMAND_MAP[i].length) == 0) {
            return VOICE_COMMAND_MAP[i].command;
        }
    }
    
    return VOICE_CMD_NONE;
}

void Voice_Execute_Command(VoiceCommand_t cmd)
{
    char response[64];
    
    switch (cmd) {
        case VOICE_CMD_FORWARD:
            Motor_Go_Forward(50);
            sprintf(response, "Voice: Forward 50%%\n");
            break;
            
        case VOICE_CMD_BACKWARD:
            Motor_Go_Backward(50);
            sprintf(response, "Voice: Backward 50%%\n");
            break;
            
        case VOICE_CMD_STOP:
            Motor_Force_Stop();
            sprintf(response, "Voice: Stop\n");
            break;
            
        case VOICE_CMD_RESISTANCE:
            if (Rehabilitation_Is_Active()) {
                Rehabilitation_Mode_Stop();
                sprintf(response, "Voice: Rehab mode stopped\n");
            } else {
                Rehabilitation_Mode_Start();
                sprintf(response, "Voice: Rehab mode started\n");
            }
            break;
            
        case VOICE_CMD_SPEED_UP:
            MotorStatus_t* motor = get_motor_ptr();
            if (motor->speed_percent < 90) {
                Motor_Set_Speed(motor->speed_percent + 10);
                sprintf(response, "Voice: Speed up to %d%%\n", Motor_Get_Speed());
            }
            break;
            
        case VOICE_CMD_SPEED_DOWN:
            motor = get_motor_ptr();
            if (motor->speed_percent > 10) {
                Motor_Set_Speed(motor->speed_percent - 10);
                sprintf(response, "Voice: Speed down to %d%%\n", Motor_Get_Speed());
            }
            break;
            
        case VOICE_CMD_MODE_CHANGE:
            SystemStatus_t* sys = get_system_ptr();
            if (sys->mode == MODE_MANUAL) {
                System_Change_Mode(MODE_REHABILITATION);
                sprintf(response, "Voice: Switch to rehab mode\n");
            } else {
                System_Change_Mode(MODE_MANUAL);
                sprintf(response, "Voice: Switch to manual mode\n");
            }
            break;
            
        case VOICE_CMD_EMERGENCY_STOP:
            Motor_Emergency_Brake();
            sprintf(response, "Voice: Emergency brake\n");
            break;
            
        case VOICE_CMD_TEST:
            Motor_Test_Routine();
            sprintf(response, "Voice: Test routine\n");
            break;
            
        default:
            return;  // ��������Ӧ
    }
    
    // ������Ӧ���������ڣ����ڵ��ԣ�
    SystemStatus_t* sys = get_system_ptr();
    if (sys->debug_enabled && strlen(response) > 0) {
        HAL_UART_Transmit(&huart1, (uint8_t*)response, strlen(response), 100);
    }
    
    // ������ʾ
    OLED_Force_Refresh();
}

// ============================================
// ״̬��ȡ
// ============================================

uint8_t Voice_Is_Ready(void)
{
    return g_voice.is_ready;
}

VoiceCommand_t Voice_Get_Last_Command(void)
{
    return g_voice.last_command;
}

// ============================================
// ˽�к���ʵ��
// ============================================

static void Voice_Send_Ack(void)
{
    // �򵥵�ȷ����Ӧ
    const char* ack = "OK\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)ack, strlen(ack), 100);
}

static void Voice_Reset_Buffer(void)
{
    memset(g_voice.rx_buffer, 0, sizeof(g_voice.rx_buffer));
    
    // ����DMA������
    hdma_usart2_rx.Instance->CNDTR = sizeof(g_voice.rx_buffer);
    
    // ������ܵĴ����־
    __HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_ORE | UART_FLAG_NE | 
                                 UART_FLAG_FE | UART_FLAG_PE);
}

static void Voice_Send_Config(void)
{
    // ASRPRO�����������ʵ��ģ�������
    const char* config_cmds[] = {
        // ���ò�����
        "AT+BAUD=115200\r\n",
        // ����ʶ��ģʽ
        "AT+MODE=2\r\n",
        // ����������
        "AT+SENS=70\r\n",
        // ���ӻ��Ѵʣ���ѡ��
        // "AT+WAKEUP=С��С��\r\n",
    };
    
    HAL_Delay(100);
    
    for (int i = 0; i < sizeof(config_cmds) / sizeof(config_cmds[0]); i++) {
        HAL_UART_Transmit(&huart2, (uint8_t*)config_cmds[i], strlen(config_cmds[i]), 100);
        HAL_Delay(50);
    }
    
    DEBUG_PRINT("Voice module configured\n");
}

// ============================================
// ������
// ============================================

void Voice_Handle_Error(void)
{
    if (g_voice.error_count > 10) {
        // ����̫�࣬�������³�ʼ��
        g_voice.is_ready = 0;
        HAL_Delay(100);
        
        // ֹͣ��������������
        HAL_UART_DMAStop(&huart2);
        HAL_Delay(10);
        
        Voice_Reset_Buffer();
        Voice_Start_Receive();
        
        g_voice.error_count = 0;
        g_voice.is_ready = 1;
        
        DEBUG_PRINT("Voice module reset\n");
    }
}
