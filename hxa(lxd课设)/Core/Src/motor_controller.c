#include "motor_controller.h"
#include "stm32f1xx_hal.h"
#include "encoder_reader.h"
#include "config.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

MotorStatus_t g_motor_status = {
    .direction = MOTOR_STOP,
    .speed_percent = 0,
    .pwm_duty = 0,
    .position_mm = 0.0f,
    .travel_range_mm = TRAVEL_RANGE_MM,
    .min_limit_mm = SAFETY_MARGIN_MM,
    .max_limit_mm = PHYSICAL_TRAVEL_RANGE_MM - SAFETY_MARGIN_MM,
    .soft_limit_enabled = 1,
    .emergency_stop_flag = 0,
    .last_update = 0
};

MotorConfig_t g_motor_config;

uint8_t Motor_Check_Position_Safety(float target_mm)
{
    if (target_mm < g_motor_status.min_limit_mm) {
        return 0;
    }
    if (target_mm > g_motor_status.max_limit_mm) {
        return 0;
    }
    return 1;
}

void Motor_Set_Position_Limits(float min_mm, float max_mm)
{
    g_motor_status.min_limit_mm = min_mm;
    g_motor_status.max_limit_mm = max_mm;
    
    if (g_motor_status.min_limit_mm < 0) g_motor_status.min_limit_mm = 0;
    if (g_motor_status.max_limit_mm > TRAVEL_RANGE_MM) 
        g_motor_status.max_limit_mm = TRAVEL_RANGE_MM;
    
    if (g_motor_status.max_limit_mm - g_motor_status.min_limit_mm < 10.0f) {
        g_motor_status.min_limit_mm = SAFETY_MARGIN_MM;
        g_motor_status.max_limit_mm = TRAVEL_RANGE_MM;
    }
    
    g_motor_status.travel_range_mm = g_motor_status.max_limit_mm - g_motor_status.min_limit_mm;
}

void Motor_Controller_Init(TIM_HandleTypeDef* pwm_tim, uint32_t ch1, uint32_t ch2, uint16_t pwm_period)
{
    DEBUG_PRINT("[MOTOR_INIT] ==========================================\r\n");
    DEBUG_PRINT("[MOTOR_INIT] Initializing motor controller...\r\n");
    DEBUG_PRINT("[MOTOR_INIT] PWM_TIM: %p, CH1: %lu, CH2: %lu, Period: %d\r\n", 
               pwm_tim, (unsigned long)ch1, (unsigned long)ch2, pwm_period);
    
    g_motor_config.pwm_tim = pwm_tim;
    g_motor_config.pwm_channel_1 = ch1;  // PB4 = TIM3_CH1 = PWM输出
    g_motor_config.pwm_channel_2 = ch2;  // 不再使用PB5作为PWM，改为方向控制
    g_motor_config.pwm_max_value = pwm_period;
    g_motor_config.pwm_min_value = MOTOR_PWM_MIN_DUTY;
    g_motor_config.pwm_max_limit = MOTOR_PWM_MAX_DUTY;
    g_motor_config.pb5_port = GPIOB;
    g_motor_config.pb5_pin = GPIO_PIN_5;  // PB5 = 方向控制引脚
    
    g_motor_status.travel_range_mm = TRAVEL_RANGE_MM;
    g_motor_status.soft_limit_enabled = 1;
    g_motor_status.emergency_stop_flag = 0;
    g_motor_status.direction = MOTOR_STOP;
    g_motor_status.speed_percent = 0;
    g_motor_status.pwm_duty = 0;
    
    Motor_Set_Position_Limits(SAFETY_MARGIN_MM, TRAVEL_RANGE_MM);
    
    // PB5已经在tim.c中初始化为TIM3_CH2的PWM功能
    // 但我们需要将其改为普通GPIO输出模式，作为方向控制引脚
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    // 使能GPIOB时钟
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    // 重新初始化PB5为普通GPIO输出模式
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    DEBUG_PRINT("[MOTOR_INIT] PB5 GPIO initialization completed\r\n");
    
    // 初始方向：正转（PB5低电平）
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
    DEBUG_PRINT("[MOTOR_INIT] PB5 initialized as direction control (LOW=Forward, HIGH=Backward)\r\n");
    DEBUG_PRINT("[MOTOR_INIT] Initial PB5 state: %d\r\n", (GPIOB->IDR & GPIO_PIN_5) != 0);
    
    // 先启动PWM通道1（PB4），然后设置占空比为0
    DEBUG_PRINT("[MOTOR_INIT] Starting PWM channel 1 (PB4)...\r\n");
    HAL_TIM_PWM_Start(pwm_tim, ch1);
    
    // 设置初始PWM占空比为100（测试用，确保PWM输出）
    DEBUG_PRINT("[MOTOR_INIT] Setting initial PWM duty to 100 (test)...\r\n");
    __HAL_TIM_SET_COMPARE(pwm_tim, ch1, 100);
    
    // 强制更新TIM3的寄存器
    pwm_tim->Instance->EGR = TIM_EGR_UG;
    
    DEBUG_PRINT("[MOTOR_INIT] Motor Controller initialized successfully!\r\n");
    DEBUG_PRINT("[MOTOR_INIT] PWM Period: %d, Max Duty: %d\r\n", pwm_period, MOTOR_PWM_MAX_DUTY);
    DEBUG_PRINT("[MOTOR_INIT] ==========================================\r\n");
}

void Motor_Set_Direction(MotorDirection_t dir)
{
    DEBUG_PRINT("[MOTOR] >>> Motor_Set_Direction(%d) CALLED <<<\r\n", dir);
    DEBUG_PRINT("[MOTOR] emergency_stop_flag: %d\r\n", g_motor_status.emergency_stop_flag);
    DEBUG_PRINT("[MOTOR] Before direction change, PB5 state: %d\r\n", (GPIOB->IDR & GPIO_PIN_5) != 0);
    
    if (g_motor_status.emergency_stop_flag && dir != MOTOR_STOP && dir != MOTOR_BRAKE) {
        DEBUG_PRINT("[MOTOR] Emergency stop active, ignoring direction change\r\n");
        return;
    }
    
    g_motor_status.direction = dir;
    g_motor_status.last_update = HAL_GetTick();
    
    // 确保PWM已启动（只需要通道1）
    HAL_TIM_PWM_Start(g_motor_config.pwm_tim, g_motor_config.pwm_channel_1);
    
    switch(dir) {
        case MOTOR_FORWARD:
            DEBUG_PRINT("[MOTOR] Forward - PWM=%d, PB5=LOW\r\n", g_motor_status.pwm_duty);
            // PB5低电平表示正转（直接操作寄存器）
            GPIOB->BRR = GPIO_PIN_5;
            // PB4输出PWM
            __HAL_TIM_SET_COMPARE(g_motor_config.pwm_tim, g_motor_config.pwm_channel_1, g_motor_status.pwm_duty);
            DEBUG_PRINT("[MOTOR] After Forward, PB5 state: %d\r\n", (GPIOB->IDR & GPIO_PIN_5) != 0);
            break;
            
        case MOTOR_BACKWARD:
            DEBUG_PRINT("[MOTOR] Backward - PWM=%d, PB5=HIGH\r\n", g_motor_status.pwm_duty);
            // PB5高电平表示反转（直接操作寄存器）
            GPIOB->BSRR = GPIO_PIN_5;
            // PB4输出PWM
            __HAL_TIM_SET_COMPARE(g_motor_config.pwm_tim, g_motor_config.pwm_channel_1, g_motor_status.pwm_duty);
            DEBUG_PRINT("[MOTOR] After Backward, PB5 state: %d\r\n", (GPIOB->IDR & GPIO_PIN_5) != 0);
            break;
            
        case MOTOR_STOP:
            DEBUG_PRINT("[MOTOR] Stop - PWM=0, PB5=LOW\r\n");
            // PB5低电平（直接操作寄存器）
            GPIOB->BRR = GPIO_PIN_5;
            __HAL_TIM_SET_COMPARE(g_motor_config.pwm_tim, g_motor_config.pwm_channel_1, 0);
            DEBUG_PRINT("[MOTOR] After Stop, PB5 state: %d\r\n", (GPIOB->IDR & GPIO_PIN_5) != 0);
            break;
            
        case MOTOR_BRAKE:
            DEBUG_PRINT("[MOTOR] Brake - PWM=%d, PB5=LOW\r\n", g_motor_config.pwm_max_value);
            // PB5低电平（直接操作寄存器）
            GPIOB->BRR = GPIO_PIN_5;
            __HAL_TIM_SET_COMPARE(g_motor_config.pwm_tim, g_motor_config.pwm_channel_1, g_motor_config.pwm_max_value);
            DEBUG_PRINT("[MOTOR] After Brake, PB5 state: %d\r\n", (GPIOB->IDR & GPIO_PIN_5) != 0);
            break;
            
        default:
            DEBUG_PRINT("[MOTOR] Invalid direction: %d\r\n", dir);
            Motor_Stop();
            break;
    }
}

void Motor_Set_Speed(uint8_t speed_percent)
{
    if (speed_percent > 100) speed_percent = 100;
    
    g_motor_status.speed_percent = speed_percent;
    
    uint16_t duty = (uint16_t)((speed_percent * g_motor_config.pwm_max_limit) / 100.0f);
    
    if (duty < g_motor_config.pwm_min_value) duty = g_motor_config.pwm_min_value;
    if (duty > g_motor_config.pwm_max_limit) duty = g_motor_config.pwm_max_limit;
    
    g_motor_status.pwm_duty = duty;
    
    // 只更新PWM通道1（PB4），方向由PB5控制
    if (g_motor_status.direction == MOTOR_FORWARD || g_motor_status.direction == MOTOR_BACKWARD) {
        __HAL_TIM_SET_COMPARE(g_motor_config.pwm_tim, g_motor_config.pwm_channel_1, duty);
    }
}

void Motor_Go_Forward(uint8_t speed_percent)
{
    DEBUG_PRINT("[MOTOR] >>> Motor_Go_Forward(%d%%) CALLED <<<\r\n", speed_percent);
    Motor_Set_Direction(MOTOR_FORWARD);
    Motor_Set_Speed(speed_percent);
    DEBUG_PRINT("[MOTOR] Forward %d%% set\r\n", speed_percent);
}

void Motor_Go_Backward(uint8_t speed_percent)
{
    DEBUG_PRINT("[MOTOR] >>> Motor_Go_Backward(%d%%) CALLED <<<\r\n", speed_percent);
    DEBUG_PRINT("[MOTOR] Before Motor_Set_Direction, PB5 state: %d\r\n", (GPIOB->IDR & GPIO_PIN_5) != 0);
    Motor_Set_Direction(MOTOR_BACKWARD);
    DEBUG_PRINT("[MOTOR] After Motor_Set_Direction, PB5 state: %d\r\n", (GPIOB->IDR & GPIO_PIN_5) != 0);
    Motor_Set_Speed(speed_percent);
    DEBUG_PRINT("[MOTOR] Backward %d%% set\r\n", speed_percent);
}

void Motor_Stop(void)
{
    DEBUG_PRINT("[MOTOR] ==========================================\r\n");
    DEBUG_PRINT("[MOTOR] >>> Motor_Stop() CALLED <<<\r\n");
    DEBUG_PRINT("[MOTOR] Current state: dir=%d, speed=%d%%, pwm=%d\r\n", 
               g_motor_status.direction, g_motor_status.speed_percent, g_motor_status.pwm_duty);
    DEBUG_PRINT("[MOTOR] Stopping motor...\r\n");
    
    // 调用Motor_Set_Direction确保状态一致
    Motor_Set_Direction(MOTOR_STOP);
    
    // 额外保险：直接设置两个PWM通道占空比为0
    DEBUG_PRINT("[MOTOR] Setting CH1=0, CH2=0\r\n");
    __HAL_TIM_SET_COMPARE(g_motor_config.pwm_tim, g_motor_config.pwm_channel_1, 0);
    __HAL_TIM_SET_COMPARE(g_motor_config.pwm_tim, g_motor_config.pwm_channel_2, 0);
    
    // 等待稳定
    DEBUG_PRINT("[MOTOR] Waiting 1ms for stability...\r\n");
    HAL_Delay(1);
    
    g_motor_status.speed_percent = 0;
    g_motor_status.pwm_duty = 0;
    
    DEBUG_PRINT("[MOTOR] New state: dir=%d, speed=%d%%, pwm=%d\r\n", 
               g_motor_status.direction, g_motor_status.speed_percent, g_motor_status.pwm_duty);
    DEBUG_PRINT("[MOTOR] >>> Motor_Stop() COMPLETED <<<\r\n");
    DEBUG_PRINT("[MOTOR] ==========================================\r\n");
}

void Motor_Brake(void)
{
    Motor_Set_Direction(MOTOR_BRAKE);
    g_motor_status.speed_percent = 0;
    g_motor_status.pwm_duty = 0;
}

void Motor_Emergency_Stop(void)
{
    g_motor_status.emergency_stop_flag = 1;
    
    // 立即停止PWM输出
    HAL_TIM_PWM_Stop(g_motor_config.pwm_tim, g_motor_config.pwm_channel_1);
    HAL_TIM_PWM_Stop(g_motor_config.pwm_tim, g_motor_config.pwm_channel_2);
    
    // 确保引脚为低电平
    __HAL_TIM_SET_COMPARE(g_motor_config.pwm_tim, g_motor_config.pwm_channel_1, 0);
    __HAL_TIM_SET_COMPARE(g_motor_config.pwm_tim, g_motor_config.pwm_channel_2, 0);
    
    g_motor_status.direction = MOTOR_STOP;
    g_motor_status.speed_percent = 0;
    g_motor_status.pwm_duty = 0;
    
    DEBUG_PRINT("[EMERGENCY] Motor stopped!\r\n");
}

void Motor_Emergency_Brake(void)
{
    Motor_Emergency_Stop();
}

void Motor_Force_Stop(void)
{
    Motor_Stop();
}

uint8_t Motor_Is_Moving(void)
{
    return (g_motor_status.direction != MOTOR_STOP);
}

void Motor_Reset_Emergency_Stop(void)
{
    Motor_Clear_Emergency_Stop();
}

void Motor_Set_Zero_Position(void)
{
    Encoder_Set_Zero();
}

void Motor_Update_Target_Check(void)
{
    // 更新目标检查
}

void Motor_Clear_Emergency_Stop(void)
{
    g_motor_status.emergency_stop_flag = 0;
    
    // 重新启动PWM但占空比为0
    HAL_TIM_PWM_Start(g_motor_config.pwm_tim, g_motor_config.pwm_channel_1);
    HAL_TIM_PWM_Start(g_motor_config.pwm_tim, g_motor_config.pwm_channel_2);
    
    __HAL_TIM_SET_COMPARE(g_motor_config.pwm_tim, g_motor_config.pwm_channel_1, 0);
    __HAL_TIM_SET_COMPARE(g_motor_config.pwm_tim, g_motor_config.pwm_channel_2, 0);
    
    DEBUG_PRINT("[EMERGENCY] Stop cleared\r\n");
}

void Motor_Set_PWM(uint16_t duty_ch1, uint16_t duty_ch2)
{
    if (duty_ch1 > g_motor_config.pwm_max_value) duty_ch1 = g_motor_config.pwm_max_value;
    
    // 确保PWM已启动（只需要通道1）
    HAL_TIM_PWM_Start(g_motor_config.pwm_tim, g_motor_config.pwm_channel_1);
    
    // 只设置PWM通道1（PB4），通道2不再使用
    __HAL_TIM_SET_COMPARE(g_motor_config.pwm_tim, g_motor_config.pwm_channel_1, duty_ch1);
    
    // duty_ch2参数不再使用，这里可以忽略或用于其他用途
}

void Motor_Move_To_Percent(uint8_t percent, uint8_t speed)
{
    if (percent > 100) percent = 100;
    
    float target_mm = (percent / 100.0f) * g_motor_status.travel_range_mm;
    
    if (!Motor_Check_Position_Safety(target_mm)) {
        DEBUG_PRINT("[ERROR] Target position %.1fmm is out of range\r\n", target_mm);
        return;
    }
    
    Motor_Set_Speed(speed);
    
    float current_pos = Motor_Get_Position_MM();
    
    if (target_mm > current_pos) {
        Motor_Set_Direction(MOTOR_FORWARD);
    } else if (target_mm < current_pos) {
        Motor_Set_Direction(MOTOR_BACKWARD);
    } else {
        Motor_Stop();
        return;
    }
}

void Motor_Test_Routine(void)
{
    DEBUG_PRINT("[TEST] Starting motor test routine...\r\n");
    
    // 测试前进
    Motor_Set_Speed(30);
    Motor_Set_Direction(MOTOR_FORWARD);
    HAL_Delay(2000);
    
    // 停止
    Motor_Stop();
    HAL_Delay(1000);
    
    // 测试后退
    Motor_Set_Speed(30);
    Motor_Set_Direction(MOTOR_BACKWARD);
    HAL_Delay(2000);
    
    // 停止
    Motor_Stop();
    HAL_Delay(1000);
    
    DEBUG_PRINT("[TEST] Motor test routine completed\r\n");
}

void Motor_Update_Position(void)
{
    g_motor_status.position_mm = Encoder_Get_Position_MM();
    g_motor_status.last_update = HAL_GetTick();
}

void Motor_Update_Position_MM(float position_mm)
{
    g_motor_status.position_mm = position_mm;
    g_motor_status.last_update = HAL_GetTick();
}

float Motor_Get_Position_MM(void)
{
    return g_motor_status.position_mm;
}

float Motor_Get_Position_Percent(void)
{
    if (g_motor_status.travel_range_mm <= 0) return 0.0f;
    return (g_motor_status.position_mm / g_motor_status.travel_range_mm) * 100.0f;
}

uint8_t Motor_Get_Speed(void)
{
    return g_motor_status.speed_percent;
}

MotorDirection_t Motor_Get_Direction(void)
{
    return g_motor_status.direction;
}

uint8_t Motor_Is_Emergency_Stop(void)
{
    return g_motor_status.emergency_stop_flag;
}
