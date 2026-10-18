#include "rehabilitation_mode.h"
#include "stm32f1xx_hal.h"
#include "config.h"
#include <stdio.h>
#include <math.h>

RehabilitationParams_t g_rehab_params = {
    .resistance_level = REHAB_DEFAULT_LEVEL,
    .max_resistance = REHAB_MAX_RESISTANCE,
    .min_resistance = REHAB_MIN_RESISTANCE,
    .target_range_mm = REHAB_TARGET_RANGE_MM,
    .current_range_mm = 0.0f,
    .is_active = 0,
    .auto_adjust = 1,
    .last_update = 0,
    .position_min_mm = TRAVEL_RANGE_MM * 0.25f,
    .position_max_mm = TRAVEL_RANGE_MM * 0.75f,
    .current_position_mm = 0.0f,
    .center_position_mm = TRAVEL_RANGE_MM / 2.0f,
    .target_velocity = 30.0f,
    .current_velocity = 0.0f,
    .state = REHAB_MODE_IDLE
};

ImpedanceControlParams_t g_impedance_params = {
    .kp = 2.0f,
    .kd = 0.5f,
    .ki = 0.1f,
    .target_position = 0.0f,
    .current_error = 0.0f,
    .last_error = 0.0f,
    .integral_error = 0.0f,
    .output_limit = 500.0f
};

static uint8_t s_impedance_enabled = 0;


void Rehabilitation_Init(void)
{
    Rehabilitation_Mode_Init();
}

void Rehabilitation_Mode_Init(void)
{
    g_rehab_params.resistance_level = REHAB_DEFAULT_LEVEL;
    g_rehab_params.target_range_mm = REHAB_TARGET_RANGE_MM;
    g_rehab_params.is_active = 0;
    g_rehab_params.auto_adjust = 1;
    g_rehab_params.target_velocity = 30.0f;
    
    g_rehab_params.position_min_mm = TRAVEL_RANGE_MM * 0.25f;
    g_rehab_params.position_max_mm = TRAVEL_RANGE_MM * 0.75f;
    g_rehab_params.center_position_mm = TRAVEL_RANGE_MM / 2.0f;
    g_rehab_params.state = REHAB_MODE_IDLE;
    
    Impedance_Control_Init();
}

void Rehabilitation_Mode_Start(void)
{
    if (g_rehab_params.is_active) return;
    
    Motor_Stop();
    HAL_Delay(100);
    
    g_rehab_params.current_range_mm = 0.0f;
    g_rehab_params.is_active = 1;
    g_rehab_params.last_update = HAL_GetTick();
    g_rehab_params.state = REHAB_MODE_ACTIVE;
    
    uint8_t pwm_value = Rehabilitation_Calculate_Resistance();
    Motor_Set_Speed(pwm_value);
    
    char msg[64];
    sprintf(msg, "Rehab mode started (Range: %.1f-%.1f mm)\n", 
            g_rehab_params.position_min_mm, g_rehab_params.position_max_mm);
}

void Rehabilitation_Start(void)
{
    Rehabilitation_Mode_Start();
}

void Rehabilitation_Stop(void)
{
    Rehabilitation_Mode_Stop();
}

void Rehabilitation_Mode_Stop(void)
{
    if (!g_rehab_params.is_active) return;
    
    Motor_Stop();
    
    g_rehab_params.is_active = 0;
    g_rehab_params.state = REHAB_MODE_IDLE;
    
    DEBUG_PRINT("Rehab mode stopped\r\n");
}

void Rehabilitation_Set_Resistance(uint8_t level)
{
    if (level > g_rehab_params.max_resistance) level = g_rehab_params.max_resistance;
    if (level < g_rehab_params.min_resistance) level = g_rehab_params.min_resistance;
    
    g_rehab_params.resistance_level = level;
    
    if (g_rehab_params.is_active) {
        uint8_t pwm_value = Rehabilitation_Calculate_Resistance();
        Motor_Set_Speed(pwm_value);
    }
}

void Rehabilitation_Set_Range_MM(float min_mm, float max_mm)
{
    if (min_mm < 0) min_mm = 0;
    if (max_mm > TRAVEL_RANGE_MM) max_mm = TRAVEL_RANGE_MM;
    if (max_mm - min_mm < 20.0f) {
        min_mm = TRAVEL_RANGE_MM * 0.4f;
        max_mm = TRAVEL_RANGE_MM * 0.6f;
    }
    
    g_rehab_params.position_min_mm = min_mm;
    g_rehab_params.position_max_mm = max_mm;
    g_rehab_params.center_position_mm = (min_mm + max_mm) / 2.0f;
}

void Rehabilitation_Mode_Update(void)
{
    if (!g_rehab_params.is_active) return;
    
    uint32_t current_time = HAL_GetTick();
    if (current_time - g_rehab_params.last_update < RESISTANCE_RESPONSE_TIME) {
        return;
    }
    
    g_rehab_params.last_update = current_time;
    
    g_rehab_params.current_position_mm = Encoder_Get_Position_MM();
    g_rehab_params.current_velocity = Encoder_Get_Velocity_MM_S();
    
    if (s_impedance_enabled) {
        Impedance_Control_Update();
    } else {
        if (fabs(g_rehab_params.current_velocity) > MOVEMENT_DETECTION_THRESHOLD) {
            uint8_t pwm_value = Rehabilitation_Calculate_Resistance();
            
            if (g_rehab_params.current_velocity > 0) {
                Motor_Go_Backward(pwm_value);
            } else {
                Motor_Go_Forward(pwm_value);
            }
        } else {
            Motor_Stop();
        }
    }
    
    if (g_rehab_params.current_position_mm <= g_rehab_params.position_min_mm) {
        Motor_Go_Forward(g_rehab_params.resistance_level);
    } else if (g_rehab_params.current_position_mm >= g_rehab_params.position_max_mm) {
        Motor_Go_Backward(g_rehab_params.resistance_level);
    }
}

uint8_t Rehabilitation_Calculate_Resistance(void)
{
    uint8_t base_resistance = g_rehab_params.resistance_level * 10;
    
    if (base_resistance < g_rehab_params.min_resistance) {
        base_resistance = g_rehab_params.min_resistance;
    }
    if (base_resistance > g_rehab_params.max_resistance) {
        base_resistance = g_rehab_params.max_resistance;
    }
    
    return base_resistance;
}

void Impedance_Control_Init(void)
{
    g_impedance_params.kp = 200.0f;  // 刚度系数 - 位置阻力（大幅增加，确保明显对抗感）
    g_impedance_params.kd = 50.0f;   // 阻尼系数 - 速度阻力（大幅增加，确保明显对抗感）
    g_impedance_params.ki = 0.0f;    // 积分系数（暂时禁用，避免电机卡住）
    g_impedance_params.target_position = TRAVEL_RANGE_MM / 2.0f;
    g_impedance_params.current_error = 0.0f;
    g_impedance_params.output_limit = 1000.0f;  // 增加输出限幅，提高最大阻力
    
    s_impedance_enabled = 0;
    
    DEBUG_PRINT("[REHAB] Impedance Control initialized (kp=%.1f, kd=%.1f, limit=%.0f)\r\n", 
                g_impedance_params.kp, g_impedance_params.kd, g_impedance_params.output_limit);
}

void Impedance_Control_Update(void)
{
    float current_position = Encoder_Get_Position_MM();
    float current_velocity = Encoder_Get_Velocity_MM_S();
    
    // 简化的阻抗控制算法 - 避免电机卡住
    // 目标: 当用户推动电机时，产生自然的阻力感
    // 力 = 刚度系数 * 位移 + 阻尼系数 * 速度
    
    // 计算位移误差（相对于目标位置）
    g_impedance_params.current_error = g_impedance_params.target_position - current_position;
    
    // 比例项（刚度）- 位置偏离越大，阻力越大
    float proportional = g_impedance_params.kp * g_impedance_params.current_error;
    
    // 微分项（阻尼）- 速度越快，阻力越大
    // 注意：这里的符号很重要，要产生阻力，需要与速度方向相反
    float derivative = g_impedance_params.kd * (-current_velocity);
    
    // 总输出（只保留比例项和微分项）
    float output = proportional + derivative;
    
    // 输出限幅
    if (output > g_impedance_params.output_limit) output = g_impedance_params.output_limit;
    if (output < -g_impedance_params.output_limit) output = -g_impedance_params.output_limit;
    
    // 根据输出设置PWM和方向（使用直接寄存器操作）
    // 正输出: 电机正转（产生阻力），PB5低电平
    // 负输出: 电机反转（产生阻力），PB5高电平
    DEBUG_PRINT("[REHAB] Before direction control, PB5 state: %d\r\n", (GPIOB->IDR & GPIO_PIN_5) != 0);
    if (output > 5.0f) {  // 死区，防止微小振动
        uint16_t duty = (uint16_t)(output);
        // 正转: PB5低电平，PB4输出PWM（直接操作寄存器）
        GPIOB->BRR = GPIO_PIN_5;
        Motor_Set_PWM(duty, 0);
        DEBUG_PRINT("[REHAB] Forward Resistance - Duty=%d, PB5=LOW\r\n", duty);
        DEBUG_PRINT("[REHAB] After Forward, PB5 state: %d\r\n", (GPIOB->IDR & GPIO_PIN_5) != 0);
    } else if (output < -5.0f) {
        uint16_t duty = (uint16_t)(-output);
        // 反转: PB5高电平，PB4输出PWM（直接操作寄存器）
        GPIOB->BSRR = GPIO_PIN_5;
        Motor_Set_PWM(duty, 0);
        DEBUG_PRINT("[REHAB] Backward Resistance - Duty=%d, PB5=HIGH\r\n", duty);
        DEBUG_PRINT("[REHAB] After Backward, PB5 state: %d\r\n", (GPIOB->IDR & GPIO_PIN_5) != 0);
    } else {
        // 输出很小时，停止电机
        Motor_Stop();
        DEBUG_PRINT("[REHAB] Motor Stopped, PB5 state: %d\r\n", (GPIOB->IDR & GPIO_PIN_5) != 0);
    }
    
    DEBUG_PRINT("[REHAB] Pos=%.1fmm, Vel=%.1fmm/s, Error=%.1f, Output=%.1f\r\n", 
                current_position, current_velocity, g_impedance_params.current_error, output);
}

void Impedance_Set_Target_Position(float target_mm)
{
    if (target_mm < 0) target_mm = 0;
    if (target_mm > TRAVEL_RANGE_MM) target_mm = TRAVEL_RANGE_MM;
    
    g_impedance_params.target_position = target_mm;
}

void Impedance_Mode_Enable(uint8_t enable)
{
    s_impedance_enabled = enable;
    
    if (enable) {
        // 启用阻抗控制时，将当前位置设为目标位置
        g_impedance_params.target_position = Encoder_Get_Position_MM();
        g_impedance_params.integral_error = 0;
        g_impedance_params.last_error = 0;
        
        char msg[64];
        sprintf(msg, "Impedance mode enabled at %.1fmm\n", g_impedance_params.target_position);
        
        // 启动康复模式
        g_rehab_params.is_active = 1;
        g_rehab_params.state = REHAB_MODE_ACTIVE;
    } else {
        // 禁用阻抗控制
        Motor_Stop();
        g_rehab_params.is_active = 0;
        g_rehab_params.state = REHAB_MODE_IDLE;
        
        DEBUG_PRINT("Impedance mode disabled\r\n");
    }
}

void Resistance_Mode_Start(void)
{
    // 启动阻力模式
    Motor_Stop();
    HAL_Delay(50);
    
    // 设置目标位置为当前位置
    float current_pos = Encoder_Get_Position_MM();
    Impedance_Set_Target_Position(current_pos);
    
    // 启用阻抗控制
    Impedance_Mode_Enable(1);
    
    // 设置康复模式状态
    g_rehab_params.is_active = 1;
    g_rehab_params.state = REHAB_MODE_ACTIVE;
    
    char msg[64];
    sprintf(msg, "Resistance mode started at %.1fmm\n", current_pos);
}

void Resistance_Mode_Stop(void)
{
    // 停止阻力模式
    Impedance_Mode_Enable(0);
    Motor_Stop();
    
    g_rehab_params.is_active = 0;
    g_rehab_params.state = REHAB_MODE_IDLE;
    
    DEBUG_PRINT("Resistance mode stopped\r\n");
}

void Resistance_Mode_Update(void)
{
    if (!s_impedance_enabled) return;
    
    // 如果电机已经停止，不更新阻抗控制
    if (Motor_Get_Direction() == MOTOR_STOP) {
        return;
    }
    
    // 更新阻抗控制
    Impedance_Control_Update();
    
    // 检查位置限制
    float current_pos = Encoder_Get_Position_MM();
    if (current_pos < 5.0f || current_pos > TRAVEL_RANGE_MM - 5.0f) {
        // 接近边界时，增加阻力防止超出范围
        if (current_pos < 5.0f) {
            // 向前推，防止继续后退: PB5低电平，PB4输出PWM
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
            Motor_Set_PWM(50, 0);
        } else {
            // 向后推，防止继续前进: PB5高电平，PB4输出PWM
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
            Motor_Set_PWM(50, 0);
        }
    }
}

void Set_Resistance_Level(uint8_t level)
{
    // 设置阻力等级 (1-10)
    if (level < 1) level = 1;
    if (level > 10) level = 10;
    
    g_rehab_params.resistance_level = level;
    
    // 根据等级调整阻抗参数
    // 等级越高，刚度和阻尼越大
    float kp_base = 5.0f;   // 增加基础刚度系数
    float kd_base = 1.0f;   // 增加基础阻尼系数
    float ki_base = 0.2f;   // 增加基础积分系数
    
    g_impedance_params.kp = kp_base * level;
    g_impedance_params.kd = kd_base * level;
    g_impedance_params.ki = ki_base * level;
    
    // 输出限制也相应调整（增加最大输出）
    g_impedance_params.output_limit = 500.0f + (level * 50.0f);
    
    char msg[64];
    sprintf(msg, "Resistance level set to %d (kp=%.1f, kd=%.1f, limit=%.0f)\n", 
            level, g_impedance_params.kp, g_impedance_params.kd, g_impedance_params.output_limit);
}

uint8_t Rehabilitation_Is_Active(void)
{
    return g_rehab_params.is_active;
}

uint8_t Rehabilitation_Get_Level(void)
{
    return g_rehab_params.resistance_level;
}

void Rehabilitation_Increase_Level(void)
{
    if (g_rehab_params.resistance_level < 10) {
        g_rehab_params.resistance_level++;
        Set_Resistance_Level(g_rehab_params.resistance_level);
    }
}

void Rehabilitation_Decrease_Level(void)
{
    if (g_rehab_params.resistance_level > 1) {
        g_rehab_params.resistance_level--;
        Set_Resistance_Level(g_rehab_params.resistance_level);
    }
}

void Rehabilitation_Process(void)
{
    Rehabilitation_Mode_Update();
}

uint32_t Rehabilitation_Get_Exercise_Count(void)
{
    return 0;
}

float Rehabilitation_Get_Total_Distance(void)
{
    return 0.0f;
}

float Rehabilitation_Get_Min_Position(void)
{
    return g_rehab_params.position_min_mm;
}

float Rehabilitation_Get_Max_Position(void)
{
    return g_rehab_params.position_max_mm;
}

uint32_t Rehabilitation_Get_Repetitions(void)
{
    return 0;
}

uint32_t Rehabilitation_Get_Training_Time(void)
{
    return 0;
}
