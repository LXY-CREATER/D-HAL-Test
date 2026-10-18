#ifndef __REHABILITATION_H
#define __REHABILITATION_H

#include "types.h"
#include "config.h"
#include "common.h"
#include "app_globals.h"

// ============================================
// ������ģʽ����
// ============================================

// ������ȼ� (1-10)
#define REHAB_MIN_LEVEL      1
#define REHAB_MAX_LEVEL      10
#define REHAB_DEFAULT_LEVEL  3

// ������Χ (��)
#define REHAB_DEFAULT_MIN_MM    20.0f
#define REHAB_DEFAULT_MAX_MM    180.0f
#define REHAB_TARGET_RANGE_MM   160.0f

// �ƶ������
#define REHAB_MAX_VELOCITY_MM_S  100.0f
#define REHAB_MIN_VELOCITY_MM_S   5.0f

// ��������
typedef struct {
    uint8_t resistance_level;      // ������ȼ� 1-10
    uint8_t is_active;             // ������ģʽ�Ƿ����
    uint8_t auto_adjust;           // �Զ���整����
    float target_range_mm;         // Ŀ���ƶ���Χ
    float current_range_mm;        // ��ǰ�ƶ���Χ
    float position_min_mm;         // ��Сλ��
    float position_max_mm;         // ���λ��
    float center_position_mm;      // �мλ��
    float target_velocity;         // Ŀ������
    float current_velocity;        // ��ǰ����
    uint32_t last_update_time;     // �������ʱ��
    uint32_t exercise_count;       // ����ͳ��
    float total_distance_mm;       // �ܹ���
    
    // ��������
    float impedance_gain;          // ����ƫ��
    float last_position;           // ������λ��
    float last_velocity;           // ������
    float applied_force;           // �ӳ���
    uint8_t direction;             // �ƶ�����
} RehabilitationParams;

// ============================================
// ��������
// ============================================

// ��ʼ��/����
void Rehabilitation_Init(void);
void Rehabilitation_Deinit(void);

// ģʽ���ƺ���
void Rehabilitation_Start(void);
void Rehabilitation_Stop(void);
uint8_t Rehabilitation_Is_Active(void);

// ������ȼ�
void Rehabilitation_Set_Level(uint8_t level);
void Rehabilitation_Increase_Level(void);
void Rehabilitation_Decrease_Level(void);
uint8_t Rehabilitation_Get_Level(void);

// ������Χ
void Rehabilitation_Set_Range_MM(float min_mm, float max_mm);
void Rehabilitation_Set_Range_Percent(float min_pct, float max_pct);
void Rehabilitation_Get_Range_MM(float* min_mm, float* max_mm);

// ��������
void Rehabilitation_Process(void);
void Rehabilitation_Update(void);

// ��������
void Rehabilitation_Set_Impedance_Gain(float gain);
float Rehabilitation_Calculate_Resistance(float velocity, float position);
void Rehabilitation_Apply_Resistance(void);

// ������
uint32_t Rehabilitation_Get_Exercise_Count(void);
float Rehabilitation_Get_Total_Distance(void);
void Rehabilitation_Reset_Statistics(void);

// �Զ���
void Rehabilitation_Auto_Adjust(void);

#endif /* __REHABILITATION_H */
