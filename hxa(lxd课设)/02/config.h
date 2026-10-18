#ifndef __CONFIG_H
#define __CONFIG_H

#include "stm32f1xx_hal.h"

// ============================================
// Ӳ�����Ŷ���
// ============================================

// ����������ţ�AT8870��
#define MOTOR_IN1_PORT       GPIOA
#define MOTOR_IN1_PIN        GPIO_PIN_0
#define MOTOR_IN2_PORT       GPIOA  
#define MOTOR_IN2_PIN        GPIO_PIN_1

// LED״̬��
#define LED_PORT             GPIOC
#define LED_PIN              GPIO_PIN_13

// ============================================
// ϵͳ����
// ============================================
#define SYSTEM_TICK_MS       10
#define OLED_PAGE_COUNT      4

// ============================================
// ����������
// ============================================
// ����ģ��: HTPA24S3M060-B-P8 (24齿同步带轮)
// �ƶ���С: 24齿, 3M比距, 6mm带宽
// 串长: 660mm
// 导道离程: 205mm (保险计算: 200mm)
// 一轮带长 = 轮周长 = π * D = 3.1416 * 23mm = 72.26mm
// 几极盘每轮轮轮数 = 24齿 * 3 = 72齿/轮
#define ENCODER_CPR          600     // 编码器每轮轮轮数
#define GEAR_RATIO           30.0f   // 透道比
#define PULLEY_TEETH         24      // 同步带轮齿数
#define PULLEY_PITCH_MM      3.0f    // 比距
#define PULLEY_CIRCUMFERENCE_MM (PULLEY_TEETH * PULLEY_PITCH_MM)  // 24 * 3 = 72mm
#define RAIL_TRAVEL_MAX_MM   205.0f  // 导道最大离程
#define RAIL_TRAVEL_SAFE_MM  200.0f  // 安全离程
#define COUNTS_PER_MM        ((float)(ENCODER_CPR * GEAR_RATIO) / PULLEY_CIRCUMFERENCE_MM)

// 实际使用的距离参数
#define PHYSICAL_MAX_MM      RAIL_TRAVEL_SAFE_MM
#define TRAVEL_RANGE_MM      RAIL_TRAVEL_SAFE_MM

// ============================================
// PWM����
// ============================================
#define PWM_PERIOD           999
#define MIN_PWM_DUTY         50
#define MAX_PWM_DUTY         950

// ============================================
// �������
// ============================================
#define MOTOR_MAX_SPEED_PERCENT 100
#define MOTOR_MIN_SPEED_PERCENT 10

// �������ö��
typedef enum {
    MOTOR_STOP = 0,
    MOTOR_FORWARD,
    MOTOR_BACKWARD,
    MOTOR_BRAKE
} MotorDirection;

// ============================================
// ��������
// ============================================
#define BT_RX_BUFFER_SIZE    128
#define BT_TX_BUFFER_SIZE    128
#define BT_CMD_QUEUE_SIZE    20

// ��������ṹ
typedef struct {
    uint8_t command;
    uint8_t parameter;
    uint32_t timestamp;
} BluetoothCmd;

// ============================================
// ϵͳ״̬ö��
// ============================================
typedef enum {
    SYS_STATE_INIT = 0,
    SYS_STATE_IDLE,
    SYS_STATE_RUNNING,
    SYS_STATE_ERROR,
    SYS_STATE_EMERGENCY
} SystemState;

// ����ģʽö��
typedef enum {
    MODE_IDLE = 0,
    MODE_MANUAL,
    MODE_REHABILITATION,
    MODE_EMERGENCY
} OperationMode;

// ϵͳ�¼�ö��
typedef enum {
    SYS_EVENT_NONE = 0,
    SYS_EVENT_STARTUP,
    SYS_EVENT_BT_CONNECTED,
    SYS_EVENT_BT_DISCONNECTED,
    SYS_EVENT_VOICE_CMD,
    SYS_EVENT_EMERGENCY_STOP,
    SYS_EVENT_POSITION_LIMIT,
    SYS_EVENT_ERROR,
    SYS_EVENT_WARNING
} SystemEvent;

// ============================================
// ϵͳ��������
// ============================================
#ifndef DEBUG_ENABLE
#define DEBUG_ENABLE         1       // ���õ������
#endif

#ifndef DEBUG_ENABLED  
#define DEBUG_ENABLED        1       // ���ݾɴ���
#endif

// ============================================
// ����ģʽ����
// ============================================
#ifndef REHAB_DEFAULT_LEVEL
#define REHAB_DEFAULT_LEVEL  5       // Ĭ�������ȼ�
#endif

#ifndef REHAB_MIN_LEVEL
#define REHAB_MIN_LEVEL      1       // ��С�����ȼ�
#endif

#ifndef REHAB_MAX_LEVEL
#define REHAB_MAX_LEVEL      9       // ��������ȼ�
#endif

#ifndef REHAB_TARGET_RANGE_MM
#define REHAB_TARGET_RANGE_MM 100.0f // ����Ŀ�귶Χ100mm
#endif

#ifndef REHAB_VELOCITY_THRESHOLD
#define REHAB_VELOCITY_THRESHOLD 2.0f // �ٶ���ֵ(mm/s)
#endif

#ifndef REHAB_UPDATE_INTERVAL_MS
#define REHAB_UPDATE_INTERVAL_MS 20  // ����ģʽ���¼��(ms)
#endif

#ifndef SAFETY_MARGIN_MM
#define SAFETY_MARGIN_MM     5.0f    // ��ȫ�߾�5mm
#endif

#ifndef TRAVEL_RANGE_MM
#define TRAVEL_RANGE_MM      200.0f  // ��Ч�г�200mm
#endif

#ifndef PHYSICAL_MAX_MM
#define PHYSICAL_MAX_MM      205.0f  // ��������205mm
#endif

// ============================================
// ������ѧ��
// ============================================
#ifndef CLAMP
#define CLAMP(x, min, max)   ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#endif

#ifndef ABS
#define ABS(x)               ((x) < 0 ? -(x) : (x))
#endif

// ============================================
// ���������
// ============================================
#ifdef DEBUG_ENABLE
    #ifndef DEBUG_PRINT
    #include "usart.h"
    #include <stdio.h>
    #include <string.h>
    extern UART_HandleTypeDef huart1;  // �����ⲿ����
    #define DEBUG_PRINT(fmt, ...) do { \
        char debug_buf[128]; \
        snprintf(debug_buf, sizeof(debug_buf), fmt, ##__VA_ARGS__); \
        HAL_UART_Transmit(&huart1, (uint8_t*)debug_buf, strlen(debug_buf), HAL_MAX_DELAY); \
    } while(0)
    #endif
#else
    #ifndef DEBUG_PRINT
    #define DEBUG_PRINT(fmt, ...) do {} while(0)
    #endif
#endif

#endif /* __CONFIG_H */
			