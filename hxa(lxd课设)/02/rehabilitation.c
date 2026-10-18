#include "rehabilitation.h"
#include "main.h"
#include <math.h>
#include "all_includes.h"

// ============================================
// ȫ�ֱ�������
// ============================================

RehabilitationParams rehab_params = {
    .resistance_level = REHAB_DEFAULT_LEVEL,
    .is_active = 0,
    .auto_adjust = 1,
    .target_range_mm = REHAB_TARGET_RANGE_MM,
    .current_range_mm = 0,
    .position_min_mm = REHAB_DEFAULT_MIN_MM,
    .position_max_mm = REHAB_DEFAULT_MAX_MM,
    .center_position_mm = 100.0f,
    .target_velocity = 30.0f,
    .current_velocity = 0,
    .last_update_time = 0,
    .exercise_count = 0,
    .total_distance_mm = 0,
    .impedance_gain = 0.5f,
    .last_position = 0,
    .last_velocity = 0,
    .applied_force = 0,
    .direction = 0
};

// ��̬����
static float last_position_mm = 0;
static uint8_t last_direction = 0;
static uint8_t movement_detected = 0;

// �������� (��ȼ�1-10)
static const float impedance_gains[11] = {
    0.0f,   // 0 - ����
    0.2f,   // 1 - ����
    0.35f,  // 2 - ����
    0.5f,   // 3 - ����
    0.7f,   // 4 - ����
    0.9f,   // 5 - ����
    1.1f,   // 6 - ����
    1.4f,   // 7 - ����
    1.7f,   // 8 - ����
    2.0f,   // 9 - ����
    2.5f    // 10 - ����
};

// ============================================
// ��ʼ������
// ============================================

void Rehabilitation_Init(void)
{
    // ����Ĭ�Ͽ�����Χ
    rehab_params.position_min_mm = REHAB_DEFAULT_MIN_MM;
    rehab_params.position_max_mm = REHAB_DEFAULT_MAX_MM;
    rehab_params.center_position_mm = (REHAB_DEFAULT_MIN_MM + REHAB_DEFAULT_MAX_MM) / 2.0f;
    rehab_params.target_range_mm = REHAB_TARGET_RANGE_MM;
    
    // ����
    Rehabilitation_Reset_Statistics();
    
    // ����
    rehab_params.impedance_gain = impedance_gains[rehab_params.resistance_level];
    
    DEBUG_PRINT("Rehabilitation mode initialized\r\n");
    DEBUG_PRINT("Range: %.1f-%.1fmm (center: %.1fmm)\r\n",
               rehab_params.position_min_mm, rehab_params.position_max_mm,
               rehab_params.center_position_mm);
    DEBUG_PRINT("Initial resistance level: %d (gain: %.2f)\r\n",
               rehab_params.resistance_level, rehab_params.impedance_gain);
}

void Rehabilitation_Deinit(void)
{
    Rehabilitation_Stop();
    rehab_params.is_active = 0;
}

// ============================================
// ģʽ���ƺ���
// ============================================

void Rehabilitation_Start(void)
{
    if (rehab_params.is_active) return;
    
    // ֹͣ���
    Motor_Stop();
    HAL_Delay(100);
    
    // ���ò���
    rehab_params.is_active = 1;
    rehab_params.last_update_time = HAL_GetTick();
    rehab_params.current_velocity = 0;
    last_position_mm = Encoder_Get_Position_MM();
    last_direction = 0;
    movement_detected = 0;
    
    // ���㵱ǰ��Χ
    rehab_params.current_range_mm = rehab_params.position_max_mm - rehab_params.position_min_mm;
    
    // �ƶ�������λ��
    Motor_Move_To_Position_MM(rehab_params.center_position_mm, 20);
    
    DEBUG_PRINT("Rehabilitation mode STARTED\r\n");
    DEBUG_PRINT("Resistance level: %d, Range: %.1fmm\r\n",
               rehab_params.resistance_level, rehab_params.current_range_mm);
}

void Rehabilitation_Stop(void)
{
    if (!rehab_params.is_active) return;
    
    // ֹͣ���
    Motor_Stop();
    
    // ����״̬
    rehab_params.is_active = 0;
    
    DEBUG_PRINT("Rehabilitation mode STOPPED\r\n");
    DEBUG_PRINT("Exercise count: %lu, Total distance: %.1fmm\r\n",
               rehab_params.exercise_count, rehab_params.total_distance_mm);
}

uint8_t Rehabilitation_Is_Active(void)
{
    return rehab_params.is_active;
}

// ============================================
// ������ȼ�
// ============================================

void Rehabilitation_Set_Level(uint8_t level)
{
    level = CLAMP(level, REHAB_MIN_LEVEL, REHAB_MAX_LEVEL);
    
    if (rehab_params.resistance_level != level) {
        rehab_params.resistance_level = level;
        rehab_params.impedance_gain = impedance_gains[level];
        DEBUG_PRINT("Resistance level set to: %d (gain: %.2f)\r\n",
                   level, rehab_params.impedance_gain);
    }
}

void Rehabilitation_Increase_Level(void)
{
    if (rehab_params.resistance_level < REHAB_MAX_LEVEL) {
        Rehabilitation_Set_Level(rehab_params.resistance_level + 1);
    } else {
        DEBUG_PRINT("Already at maximum resistance level (%d)\r\n", REHAB_MAX_LEVEL);
    }
}

void Rehabilitation_Decrease_Level(void)
{
    if (rehab_params.resistance_level > REHAB_MIN_LEVEL) {
        Rehabilitation_Set_Level(rehab_params.resistance_level - 1);
    } else {
        DEBUG_PRINT("Already at minimum resistance level (%d)\r\n", REHAB_MIN_LEVEL);
    }
}

uint8_t Rehabilitation_Get_Level(void)
{
    return rehab_params.resistance_level;
}

// ============================================
// ������Χ
// ============================================

void Rehabilitation_Set_Range_MM(float min_mm, float max_mm)
{
    // ��֤������Ч��
    if (min_mm < 0) min_mm = 0;
    if (max_mm > RAIL_TRAVEL_SAFE_MM) max_mm = RAIL_TRAVEL_SAFE_MM;
    if (min_mm >= max_mm) return;
    
    rehab_params.position_min_mm = min_mm;
    rehab_params.position_max_mm = max_mm;
    rehab_params.center_position_mm = (min_mm + max_mm) / 2.0f;
    rehab_params.current_range_mm = max_mm - min_mm;
    
    DEBUG_PRINT("Rehab range set: %.1f-%.1fmm (center: %.1fmm)\r\n",
               min_mm, max_mm, rehab_params.center_position_mm);
}

void Rehabilitation_Set_Range_Percent(float min_pct, float max_pct)
{
    min_pct = CLAMP(min_pct, 0.0f, 100.0f);
    max_pct = CLAMP(max_pct, 0.0f, 100.0f);
    
    if (min_pct >= max_pct) return;
    
    float min_mm = (min_pct / 100.0f) * RAIL_TRAVEL_SAFE_MM;
    float max_mm = (max_pct / 100.0f) * RAIL_TRAVEL_SAFE_MM;
    
    Rehabilitation_Set_Range_MM(min_mm, max_mm);
}

void Rehabilitation_Get_Range_MM(float* min_mm, float* max_mm)
{
    if (min_mm) *min_mm = rehab_params.position_min_mm;
    if (max_mm) *max_mm = rehab_params.position_max_mm;
}

// ============================================
// ��������
// ============================================

void Rehabilitation_Set_Impedance_Gain(float gain)
{
    if (gain < 0) gain = 0;
    if (gain > 5.0f) gain = 5.0f;
    
    rehab_params.impedance_gain = gain;
}

/**
 * @brief  �����������������
 * @param  velocity: ������ (mm/s)
 * @param  position: ��ǰλ�� (mm)
 * @retval ��������PWMռ�ձ�
 * 
 * ��������: F = -K * v - B * (x - x0)
 * ������: F = -K * v - B * (x - x0)
 * K = ����ƫ��
 * B = ������
 * v = ������
 * x = ��ǰλ��
 * x0 = �мλ��
 */
float Rehabilitation_Calculate_Resistance(float velocity, float position)
{
    if (fabs(velocity) < 1.0f) {
        return 0;  // ��������
    }
    
    // ������
    float K = rehab_params.impedance_gain * 10.0f;
    
    // ������
    float B = rehab_params.impedance_gain * 0.5f;
    
    // λ��ƫ�䣨�������мλ��
    float position_offset = position - rehab_params.center_position_mm;
    
    // ��������
    // F = -K * v - B * (x - x0)
    // ������ʱ�������ƫ�����壬��������������
    float force = -K * velocity - B * position_offset;
    
    // ת��ΪPWMռ�ձ�
    // ������: 100-400 PWM (�ݴ�������ȷ���)
    float pwm_duty = fabs(force);
    
    // �������PWM��Χ
    if (pwm_duty < 50) pwm_duty = 50;
    if (pwm_duty > 400) pwm_duty = 400;
    
    // ����ʱ������ת���������
    // ������: velocity > 0 (ǰ��) -> ������Ҫ�������� (����)
    // ������: velocity < 0 (����) -> ������Ҫ�������� (ǰ��)
    rehab_params.direction = (velocity > 0) ? 1 : 0;
    
    return pwm_duty;
}

void Rehabilitation_Apply_Resistance(void)
{
    float current_position = Encoder_Get_Position_MM();
    float current_velocity = Encoder_Get_Velocity_MM_S();
    
    // �������
    float resistance_pwm = Rehabilitation_Calculate_Resistance(current_velocity, current_position);
    
    // �ж��Ƿ�������
    if (fabs(current_velocity) > REHAB_MIN_VELOCITY_MM_S) {
        // �����壬���������ƫ�����壬��������������
        // ������: velocity > 0 (ǰ��) -> ������Ҫ�������� (����)
        // ������: velocity < 0 (����) -> ������Ҫ�������� (ǰ��)
        
        if (current_velocity > 0) {
            // ������ǰ������������
            AT8870_Set_State(AT8870_BACKWARD, (uint16_t)resistance_pwm);
        } else {
            // ������ִ�����������
            AT8870_Set_State(AT8870_FORWARD, (uint16_t)resistance_pwm);
        }
        
        movement_detected = 1;
        rehab_params.applied_force = resistance_pwm;
    } else {
        // �����壬����ֹͣ
        if (movement_detected) {
            AT8870_Stop();
            movement_detected = 0;
        }
        rehab_params.applied_force = 0;
    }
    
    // �����λ��
    rehab_params.last_position = current_position;
    rehab_params.last_velocity = current_velocity;
}

// ============================================
// ��������
// ============================================

void Rehabilitation_Update(void)
{
    if (!rehab_params.is_active) return;
    
    uint32_t current_time = HAL_GetTick();
    uint32_t dt = current_time - rehab_params.last_update_time;
    
    if (dt < 10) return;  // 10ms����
    
    float current_position = Encoder_Get_Position_MM();
    float current_velocity = Encoder_Get_Velocity_MM_S();
    
    // �ƶ���
    float delta_position = fabs(current_position - last_position_mm);
    
    // ���ƶ�����
    uint8_t current_dir = (current_velocity > 0) ? 1 : 0;
    
    // �ж��Ƿ������ת��
    if (last_direction != 0 && current_dir != last_direction && 
        fabs(current_velocity) > REHAB_MIN_VELOCITY_MM_S) {
        // �ת��������
        rehab_params.exercise_count++;
        DEBUG_PRINT("Exercise count: %lu\r\n", rehab_params.exercise_count);
    }
    
    // �ܹ���
    rehab_params.total_distance_mm += delta_position;
    
    // �����λ��
    last_position_mm = current_position;
    last_direction = current_dir;
    rehab_params.last_update_time = current_time;
    rehab_params.current_velocity = current_velocity;
}

void Rehabilitation_Process(void)
{
    if (!rehab_params.is_active) return;
    
    // �������
    Rehabilitation_Update();
    
    // ��������
    Rehabilitation_Apply_Resistance();
    
    // �Զ���
    if (rehab_params.auto_adjust) {
        Rehabilitation_Auto_Adjust();
    }
}

// ============================================
// �Զ���
// ============================================

void Rehabilitation_Auto_Adjust(void)
{
    // ������ڹ����������Զ���整����
    // ������ʱ������
    // ������ʱ������
    
    float velocity = fabs(Encoder_Get_Velocity_MM_S());
    
    if (velocity > REHAB_MAX_VELOCITY_MM_S * 0.8f) {
        // �ٶ�̫�٣������
        if (rehab_params.resistance_level < REHAB_MAX_LEVEL) {
            // ������
        }
    } else if (velocity < REHAB_MAX_VELOCITY_MM_S * 0.2f) {
        // �ٶ�̫�������
        if (rehab_params.resistance_level > REHAB_MIN_LEVEL) {
            // ������
        }
    }
}

// ============================================
// ������
// ============================================

uint32_t Rehabilitation_Get_Exercise_Count(void)
{
    return rehab_params.exercise_count;
}

float Rehabilitation_Get_Total_Distance(void)
{
    return rehab_params.total_distance_mm;
}

void Rehabilitation_Reset_Statistics(void)
{
    rehab_params.exercise_count = 0;
    rehab_params.total_distance_mm = 0;
    last_position_mm = 0;
    last_direction = 0;
}
