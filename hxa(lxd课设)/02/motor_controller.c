#include "motor_controller.h"
#include "main.h"
#include <string.h>
#include <math.h>

// ============================================
// 私有函数
// ============================================

// 使用访问函数获取全局状态
static MotorState* get_motor_ptr(void) {
    return get_motor_state();
}

static MotorConfig* get_config_ptr(void) {
    return get_motor_config();
}

// ============================================
// AT8870驱动函数
// ============================================

// 设置AT8870输出状态
void AT8870_Set_Output(uint8_t in1, uint8_t in2)
{
    HAL_GPIO_WritePin(MOTOR_IN1_PORT, MOTOR_IN1_PIN, in1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_IN2_PORT, MOTOR_IN2_PIN, in2 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// 安全停止AT8870
void AT8870_Safe_Stop(void)
{
    MotorConfig* config = get_config_ptr();
    
    // 1. 停止PWM输出
    if (config->pwm_tim) {
        HAL_TIM_PWM_Stop(config->pwm_tim, config->pwm_channel_1);
        HAL_TIM_PWM_Stop(config->pwm_tim, config->pwm_channel_2);
    }
    
    // 2. 重新配置GPIO为普通输出模式
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 配置IN1引脚
    GPIO_InitStruct.Pin = MOTOR_IN1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MOTOR_IN1_PORT, &GPIO_InitStruct);
    
    // 配置IN2引脚
    GPIO_InitStruct.Pin = MOTOR_IN2_PIN;
    HAL_GPIO_Init(MOTOR_IN2_PORT, &GPIO_InitStruct);
    
    // 3. 输出低电平
    HAL_GPIO_WritePin(MOTOR_IN1_PORT, MOTOR_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_IN2_PORT, MOTOR_IN2_PIN, GPIO_PIN_RESET);
    
    // 4. 短暂延时
    HAL_Delay(10);
    
    // 5. 重新配置为PWM模式
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;  // 复用推挽输出
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    
    GPIO_InitStruct.Pin = MOTOR_IN1_PIN;
    HAL_GPIO_Init(MOTOR_IN1_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = MOTOR_IN2_PIN;
    HAL_GPIO_Init(MOTOR_IN2_PORT, &GPIO_InitStruct);
    
    // 6. 重新启动PWM（占空比为0）
    if (config->pwm_tim) {
        HAL_TIM_PWM_Start(config->pwm_tim, config->pwm_channel_1);
        HAL_TIM_PWM_Start(config->pwm_tim, config->pwm_channel_2);
        __HAL_TIM_SET_COMPARE(config->pwm_tim, config->pwm_channel_1, 0);
        __HAL_TIM_SET_COMPARE(config->pwm_tim, config->pwm_channel_2, 0);
    }
    
    // 7. 更新状态
    MotorState* motor = get_motor_ptr();
    motor->direction = MOTOR_STOP;
    motor->speed_percent = 0;
    motor->pwm_duty = 0;
    motor->is_moving = 0;
}

// ============================================
// 初始化函数
// ============================================

// 初始化电机控制器
void Motor_Init(TIM_HandleTypeDef* pwm_tim, uint32_t ch1, uint32_t ch2)
{
    MotorConfig* config = get_config_ptr();
    MotorState* motor = get_motor_ptr();
    
    // 保存配置
    config->pwm_tim = pwm_tim;
    config->pwm_channel_1 = ch1;
    config->pwm_channel_2 = ch2;
    config->pwm_period = __HAL_TIM_GET_AUTORELOAD(pwm_tim);
    
    // 确保PWM停止
    HAL_TIM_PWM_Stop(pwm_tim, ch1);
    HAL_TIM_PWM_Stop(pwm_tim, ch2);
    
    // 安全停止电机
    AT8870_Safe_Stop();
    
    // 设置默认限位
    motor->min_limit_mm = SAFETY_MARGIN_MM;
    motor->max_limit_mm = TRAVEL_RANGE_MM - SAFETY_MARGIN_MM;
    motor->travel_range_mm = motor->max_limit_mm - motor->min_limit_mm;
    
    // 启动PWM
    HAL_TIM_PWM_Start(pwm_tim, ch1);
    HAL_TIM_PWM_Start(pwm_tim, ch2);
    
    DEBUG_PRINT("Motor controller initialized. PWM period: %d\r\n", config->pwm_period);
}

// 反初始化电机控制器
void Motor_Deinit(void)
{
    Motor_Force_Stop();
    
    MotorConfig* config = get_config_ptr();
    
    if (config->pwm_tim) {
        HAL_TIM_PWM_Stop(config->pwm_tim, config->pwm_channel_1);
        HAL_TIM_PWM_Stop(config->pwm_tim, config->pwm_channel_2);
    }
    
    config->pwm_tim = NULL;
}

// ============================================
// 基本控制函数
// ============================================

// 设置电机方向
void Motor_Set_Direction(uint8_t dir)
{
    MotorState* motor = get_motor_ptr();
    MotorConfig* config = get_config_ptr();
    
    // 如果紧急停止激活，只允许停止或刹车
    if (motor->emergency_stop && dir != MOTOR_STOP && dir != MOTOR_BRAKE) {
        return;
    }
    
    // 如果方向相同且不是停止/刹车，直接返回
    if (dir == motor->direction && dir != MOTOR_STOP && dir != MOTOR_BRAKE) {
        return;
    }
    
    motor->direction = dir;
    motor->is_moving = (dir != MOTOR_STOP && dir != MOTOR_BRAKE);
    
    switch (dir) {
        case MOTOR_FORWARD:
            // IN1 = PWM, IN2 = 0
            __HAL_TIM_SET_COMPARE(config->pwm_tim, config->pwm_channel_2, 0);
            __HAL_TIM_SET_COMPARE(config->pwm_tim, config->pwm_channel_1, motor->pwm_duty);
            AT8870_Set_Output(1, 0);
            DEBUG_PRINT("Motor: FORWARD, PWM=%d\r\n", motor->pwm_duty);
            break;
            
        case MOTOR_BACKWARD:
            // IN1 = 0, IN2 = PWM
            __HAL_TIM_SET_COMPARE(config->pwm_tim, config->pwm_channel_1, 0);
            __HAL_TIM_SET_COMPARE(config->pwm_tim, config->pwm_channel_2, motor->pwm_duty);
            AT8870_Set_Output(0, 1);
            DEBUG_PRINT("Motor: BACKWARD, PWM=%d\r\n", motor->pwm_duty);
            break;
            
        case MOTOR_STOP:
            AT8870_Safe_Stop();
            motor->speed_percent = 0;
            motor->pwm_duty = 0;
            DEBUG_PRINT("Motor: STOP\r\n");
            break;
            
        case MOTOR_BRAKE:
            // IN1 = 1, IN2 = 1
            __HAL_TIM_SET_COMPARE(config->pwm_tim, config->pwm_channel_1, config->pwm_period);
            __HAL_TIM_SET_COMPARE(config->pwm_tim, config->pwm_channel_2, config->pwm_period);
            AT8870_Set_Output(1, 1);
            motor->is_moving = 0;
            
            // 刹车保持50ms后回到停止状态
            HAL_Delay(50);
            Motor_Set_Direction(MOTOR_STOP);
            DEBUG_PRINT("Motor: BRAKE\r\n");
            break;
    }
    
    motor->last_update_time = HAL_GetTick();
}

// 设置电机速度
void Motor_Set_Speed(uint8_t percent)
{
    MotorState* motor = get_motor_ptr();
    MotorConfig* config = get_config_ptr();
    
    // 限制速度范围
    percent = CLAMP(percent, 0, MOTOR_MAX_SPEED_PERCENT);
    
    // 如果速度为0，直接停止
    if (percent == 0) {
        motor->speed_percent = 0;
        motor->pwm_duty = 0;
        if (motor->direction != MOTOR_STOP) {
            Motor_Set_Direction(MOTOR_STOP);
        }
        return;
    }
    
    // 如果速度非0但低于最小值，设置为最小值
    if (percent > 0 && percent < MOTOR_MIN_SPEED_PERCENT) {
        percent = MOTOR_MIN_SPEED_PERCENT;
    }
    
    // 计算PWM占空比
    uint16_t pwm_value = 0;
    if (percent > 0) {
        pwm_value = config->pwm_min_duty + 
                   (percent * (config->pwm_max_duty - config->pwm_min_duty)) / 100;
    }
    
    motor->speed_percent = percent;
    motor->pwm_duty = pwm_value;
    
    // 更新当前方向的PWM
    if (motor->direction == MOTOR_FORWARD) {
        __HAL_TIM_SET_COMPARE(config->pwm_tim, config->pwm_channel_1, pwm_value);
    } else if (motor->direction == MOTOR_BACKWARD) {
        __HAL_TIM_SET_COMPARE(config->pwm_tim, config->pwm_channel_2, pwm_value);
    }
    
    DEBUG_PRINT("Motor speed: %d%%, PWM=%d\r\n", percent, pwm_value);
}

// 停止电机
void Motor_Stop(void)
{
    Motor_Set_Direction(MOTOR_STOP);
}

// 刹车
void Motor_Brake(void)
{
    Motor_Set_Direction(MOTOR_BRAKE);
}

// 紧急刹车
void Motor_Emergency_Brake(void)
{
    MotorState* motor = get_motor_ptr();
    MotorConfig* config = get_config_ptr();
    
    motor->emergency_stop = 1;
    
    // 1. 立即刹车
    __HAL_TIM_SET_COMPARE(config->pwm_tim, config->pwm_channel_1, config->pwm_period);
    __HAL_TIM_SET_COMPARE(config->pwm_tim, config->pwm_channel_2, config->pwm_period);
    AT8870_Set_Output(1, 1);
    
    // 2. 保持刹车100ms
    HAL_Delay(100);
    
    // 3. 完全停止
    AT8870_Safe_Stop();
    
    // 4. 更新状态
    motor->direction = MOTOR_STOP;
    motor->speed_percent = 0;
    motor->pwm_duty = 0;
    motor->is_moving = 0;
    
    DEBUG_PRINT("Motor: EMERGENCY BRAKE!\r\n");
}

// 强制停止（最彻底的停止）
void Motor_Force_Stop(void)
{
    MotorState* motor = get_motor_ptr();
    MotorConfig* config = get_config_ptr();
    
    motor->emergency_stop = 1;
    
    // 1. 停止PWM
    if (config->pwm_tim) {
        HAL_TIM_PWM_Stop(config->pwm_tim, config->pwm_channel_1);
        HAL_TIM_PWM_Stop(config->pwm_tim, config->pwm_channel_2);
    }
    
    // 2. 重新配置GPIO为普通输出
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 配置IN1引脚
    GPIO_InitStruct.Pin = MOTOR_IN1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MOTOR_IN1_PORT, &GPIO_InitStruct);
    
    // 配置IN2引脚
    GPIO_InitStruct.Pin = MOTOR_IN2_PIN;
    HAL_GPIO_Init(MOTOR_IN2_PORT, &GPIO_InitStruct);
    
    // 3. 输出低电平
    HAL_GPIO_WritePin(MOTOR_IN1_PORT, MOTOR_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_IN2_PORT, MOTOR_IN2_PIN, GPIO_PIN_RESET);
    
    // 4. 短暂延时
    HAL_Delay(10);
    
    // 5. 更新状态
    motor->direction = MOTOR_STOP;
    motor->speed_percent = 0;
    motor->pwm_duty = 0;
    motor->is_moving = 0;
    
    DEBUG_PRINT("Motor: FORCE STOP!\r\n");
}

// 平滑停止
void Motor_Smooth_Stop(void)
{
    MotorState* motor = get_motor_ptr();
    
    if (motor->speed_percent == 0) return;
    
    uint8_t current_speed = motor->speed_percent;
    
    // 逐渐降低速度
    for (int i = current_speed; i >= 0; i -= 10) {
        if (i < 10) i = 0;
        Motor_Set_Speed(i);
        HAL_Delay(20);
    }
    
    Motor_Stop();
    DEBUG_PRINT("Motor: SMOOTH STOP\r\n");
}

// ============================================
// 运动控制函数
// ============================================

// 前进
void Motor_Go_Forward(uint8_t speed)
{
    MotorState* motor = get_motor_ptr();
    
    if (motor->emergency_stop) {
        DEBUG_PRINT("Cannot move: Emergency stop active\r\n");
        return;
    }
    
    // 位置安全检查
    if (motor->soft_limit_enabled) {
        float current_mm = motor->position_mm;
        float distance_to_max = motor->max_limit_mm - current_mm;
        
        if (distance_to_max <= 0) {
            DEBUG_PRINT("Already at max limit: %.1fmm\r\n", current_mm);
            Motor_Stop();
            return;
        } else if (distance_to_max < 10.0f) {
            // 接近上限，自动减速
            if (speed > 30) speed = 30;
        } else if (distance_to_max < 5.0f) {
            // 非常接近上限，强制停止
            Motor_Smooth_Stop();
            return;
        }
    }
    
    Motor_Set_Direction(MOTOR_FORWARD);
    Motor_Set_Speed(speed);
}

// 后退
void Motor_Go_Backward(uint8_t speed)
{
    MotorState* motor = get_motor_ptr();
    
    if (motor->emergency_stop) {
        DEBUG_PRINT("Cannot move: Emergency stop active\r\n");
        return;
    }
    
    // 位置安全检查
    if (motor->soft_limit_enabled) {
        float current_mm = motor->position_mm;
        float distance_to_min = current_mm - motor->min_limit_mm;
        
        if (distance_to_min <= 0) {
            DEBUG_PRINT("Already at min limit: %.1fmm\r\n", current_mm);
            Motor_Stop();
            return;
        } else if (distance_to_min < 10.0f) {
            // 接近下限，自动减速
            if (speed > 30) speed = 30;
        } else if (distance_to_min < 5.0f) {
            // 非常接近下限，强制停止
            Motor_Smooth_Stop();
            return;
        }
    }
    
    Motor_Set_Direction(MOTOR_BACKWARD);
    Motor_Set_Speed(speed);
}

// 移动到指定位置（毫米）
void Motor_Move_To_Position_MM(float position_mm, uint8_t speed)
{
    MotorState* motor = get_motor_ptr();
    
    if (motor->emergency_stop) return;
    
    // 位置安全检查
    float current_mm = motor->position_mm;
    
    if (motor->soft_limit_enabled) {
        if (position_mm < motor->min_limit_mm || position_mm > motor->max_limit_mm) {
            DEBUG_PRINT("Target position %.1fmm is out of safe range!\r\n", position_mm);
            return;
        }
    }
    
    float distance = position_mm - current_mm;
    
    // 如果距离很小，直接停止
    if (fabs(distance) < 0.5f) {
        Motor_Stop();
        return;
    }
    
    // 动态调整速度
    if (fabs(distance) < 10.0f) {
        // 接近目标，减速
        if (speed > 30) speed = 30;
    }
    
    // 设置目标位置
    motor->target_position_mm = position_mm;
    motor->target_position_percent = ((position_mm - motor->min_limit_mm) / 
                                       motor->travel_range_mm) * 100.0f;
    motor->is_moving = 1;
    
    // 决定运动方向
    if (distance > 0) {
        Motor_Go_Forward(speed);
    } else {
        Motor_Go_Backward(speed);
    }
    
    DEBUG_PRINT("Moving to %.1fmm (current: %.1fmm)\r\n", position_mm, current_mm);
}

// 移动到百分比位置
void Motor_Move_To_Percent(float percent, uint8_t speed)
{
    MotorState* motor = get_motor_ptr();
    
    percent = CLAMP(percent, 0, 100);
    
    // 计算目标位置（毫米）
    float target_mm = motor->min_limit_mm + 
                     (percent / 100.0f) * motor->travel_range_mm;
    
    Motor_Move_To_Position_MM(target_mm, speed);
}

// 移动到中心位置
void Motor_Move_To_Center(uint8_t speed)
{
    MotorState* motor = get_motor_ptr();
    float center_mm = motor->min_limit_mm + motor->travel_range_mm / 2.0f;
    Motor_Move_To_Position_MM(center_mm, speed);
}

// ============================================
// 位置管理函数
// ============================================

// 设置位置限制
void Motor_Set_Position_Limits(float min_mm, float max_mm)
{
    MotorState* motor = get_motor_ptr();
    
    min_mm = CLAMP(min_mm, 0, TRAVEL_RANGE_MM);
    max_mm = CLAMP(max_mm, 0, TRAVEL_RANGE_MM);
    
    // 确保min < max
    if (min_mm >= max_mm) {
        min_mm = SAFETY_MARGIN_MM;
        max_mm = TRAVEL_RANGE_MM - SAFETY_MARGIN_MM;
    }
    
    motor->min_limit_mm = min_mm;
    motor->max_limit_mm = max_mm;
    motor->travel_range_mm = max_mm - min_mm;
    
    DEBUG_PRINT("Position limits set: %.1f-%.1fmm (range: %.1fmm)\r\n",
               min_mm, max_mm, motor->travel_range_mm);
}

// 启用/禁用软限位
void Motor_Enable_Soft_Limit(uint8_t enable)
{
    MotorState* motor = get_motor_ptr();
    motor->soft_limit_enabled = enable;
    DEBUG_PRINT("Soft limit: %s\r\n", enable ? "ENABLED" : "DISABLED");
}

// 启用/禁用自动停止
void Motor_Enable_Auto_Stop(uint8_t enable)
{
    MotorState* motor = get_motor_ptr();
    motor->auto_stop_enabled = enable;
    DEBUG_PRINT("Auto stop: %s\r\n", enable ? "ENABLED" : "DISABLED");
}

// 检查位置安全性
uint8_t Motor_Check_Position_Safety(float target_mm)
{
    MotorState* motor = get_motor_ptr();
    
    if (!motor->soft_limit_enabled) return 1;
    
    if (target_mm < motor->min_limit_mm || target_mm > motor->max_limit_mm) {
        return 0;
    }
    return 1;
}

// 设置零点位置
void Motor_Set_Zero_Position(void)
{
    MotorState* motor = get_motor_ptr();
    
    motor->position_mm = 0;
    motor->position_percent = 0;
    motor->calibrated = 1;
    
    DEBUG_PRINT("Zero position set\r\n");
}

// 校准电机
void Motor_Calibrate(void)
{
    MotorState* motor = get_motor_ptr();
    
    DEBUG_PRINT("Starting motor calibration...\r\n");
    
    // 1. 移动到中间位置
    float center_mm = motor->min_limit_mm + motor->travel_range_mm / 2.0f;
    Motor_Move_To_Position_MM(center_mm, 20);
    
    // 2. 等待到达
    uint32_t start_time = HAL_GetTick();
    while (motor->is_moving && (HAL_GetTick() - start_time < 5000)) {
        HAL_Delay(10);
    }
    
    // 3. 设置当前位置为50%
    motor->position_mm = center_mm;
    motor->position_percent = 50.0f;
    motor->calibrated = 1;
    
    DEBUG_PRINT("Motor calibration complete. Center: %.1fmm\r\n", center_mm);
}

// 重置紧急停止
void Motor_Reset_Emergency_Stop(void)
{
    MotorState* motor = get_motor_ptr();
    motor->emergency_stop = 0;
    DEBUG_PRINT("Emergency stop reset\r\n");
}

// ============================================
// 状态更新函数
// ============================================

// 更新位置
void Motor_Update_Position(float current_mm)
{
    MotorState* motor = get_motor_ptr();
    
    motor->position_mm = current_mm;
    
    // 计算百分比位置
    if (motor->travel_range_mm > 0) {
        motor->position_percent = ((current_mm - motor->min_limit_mm) / 
                                    motor->travel_range_mm) * 100.0f;
        motor->position_percent = CLAMP(motor->position_percent, 0, 100);
    }
    
    motor->last_update_time = HAL_GetTick();
}

// 更新目标检查
void Motor_Update_Target_Check(void)
{
    MotorState* motor = get_motor_ptr();
    
    if (!motor->auto_stop_enabled || !motor->is_moving) return;
    
    // 检查是否到达目标位置
    if (motor->target_position_mm > 0) {
        float error = fabs(motor->target_position_mm - motor->position_mm);
        if (error < 0.5f) {
            Motor_Stop();
            DEBUG_PRINT("Target position reached: %.1fmm\r\n", motor->target_position_mm);
        }
    }
    
    // 检查是否接近限位
    if (motor->soft_limit_enabled) {
        float current_mm = motor->position_mm;
        
        if (motor->direction == MOTOR_FORWARD) {
            if (current_mm >= motor->max_limit_mm - 1.0f) {
                Motor_Smooth_Stop();
                DEBUG_PRINT("Auto stop near max limit: %.1fmm\r\n", current_mm);
            }
        } else if (motor->direction == MOTOR_BACKWARD) {
            if (current_mm <= motor->min_limit_mm + 1.0f) {
                Motor_Smooth_Stop();
                DEBUG_PRINT("Auto stop near min limit: %.1fmm\r\n", current_mm);
            }
        }
    }
}

// ============================================
// 状态获取函数
// ============================================

MotorDirection Motor_Get_Direction(void)
{
    MotorState* motor = get_motor_ptr();
    return (MotorDirection)motor->direction;
}

uint8_t Motor_Get_Speed(void)
{
    MotorState* motor = get_motor_ptr();
    return motor->speed_percent;
}

float Motor_Get_Position_MM(void)
{
    MotorState* motor = get_motor_ptr();
    return motor->position_mm;
}

float Motor_Get_Position_Percent(void)
{
    MotorState* motor = get_motor_ptr();
    return motor->position_percent;
}

uint8_t Motor_Is_Moving(void)
{
    MotorState* motor = get_motor_ptr();
    return motor->is_moving;
}

uint8_t Motor_Is_At_Limit(void)
{
    MotorState* motor = get_motor_ptr();
    if (motor->position_mm <= motor->min_limit_mm + 0.5f) return 1;
    if (motor->position_mm >= motor->max_limit_mm - 0.5f) return 2;
    return 0;
}

uint8_t Motor_Is_Emergency_Stop(void)
{
    MotorState* motor = get_motor_ptr();
    return motor->emergency_stop;
}

// ============================================
// 测试函数
// ============================================

// 电机测试程序
void Motor_Test_Routine(void)
{
    MotorState* motor = get_motor_ptr();
    
    DEBUG_PRINT("Starting motor test routine...\r\n");
    
    // 保存当前状态
    uint8_t saved_soft_limit = motor->soft_limit_enabled;
    uint8_t saved_auto_stop = motor->auto_stop_enabled;
    
    // 禁用限位和自动停止
    Motor_Enable_Soft_Limit(0);
    Motor_Enable_Auto_Stop(0);
    
    // 1. 前进测试
    DEBUG_PRINT("Test 1: Forward 30%%\r\n");
    Motor_Go_Forward(30);
    HAL_Delay(1000);
    Motor_Stop();
    HAL_Delay(500);
    
    // 2. 后退测试
    DEBUG_PRINT("Test 2: Backward 30%%\r\n");
    Motor_Go_Backward(30);
    HAL_Delay(1000);
    Motor_Stop();
    HAL_Delay(500);
    
    // 3. 平滑停止测试
    DEBUG_PRINT("Test 3: Smooth stop\r\n");
    Motor_Go_Forward(60);
    HAL_Delay(500);
    Motor_Smooth_Stop();
    HAL_Delay(500);
    
    // 4. 刹车测试
    DEBUG_PRINT("Test 4: Brake test\r\n");
    Motor_Go_Backward(50);
    HAL_Delay(300);
    Motor_Brake();
    HAL_Delay(500);
    
    // 5. 紧急刹车测试
    DEBUG_PRINT("Test 5: Emergency brake\r\n");
    Motor_Go_Forward(40);
    HAL_Delay(200);
    Motor_Emergency_Brake();
    HAL_Delay(1000);
    
    // 重置紧急停止
    Motor_Reset_Emergency_Stop();
    
    // 6. 返回到中心位置
    DEBUG_PRINT("Test 6: Return to center\r\n");
    Motor_Move_To_Center(30);
    
    // 等待到达中心
    uint32_t start_time = HAL_GetTick();
    while (motor->is_moving && (HAL_GetTick() - start_time < 5000)) {
        HAL_Delay(10);
    }
    
    // 恢复设置
    Motor_Enable_Soft_Limit(saved_soft_limit);
    Motor_Enable_Auto_Stop(saved_auto_stop);
    
    DEBUG_PRINT("Motor test routine completed\r\n");
}

// 检查引脚状态
void Motor_Check_Pin_State(void)
{
    MotorState* motor = get_motor_ptr();
    MotorConfig* config = get_config_ptr();
    
    GPIO_PinState pin1_state = HAL_GPIO_ReadPin(MOTOR_IN1_PORT, MOTOR_IN1_PIN);
    GPIO_PinState pin2_state = HAL_GPIO_ReadPin(MOTOR_IN2_PORT, MOTOR_IN2_PIN);
    
    uint16_t pwm1_value = 0;
    uint16_t pwm2_value = 0;
    
    if (config->pwm_tim) {
        pwm1_value = __HAL_TIM_GET_COMPARE(config->pwm_tim, config->pwm_channel_1);
        pwm2_value = __HAL_TIM_GET_COMPARE(config->pwm_tim, config->pwm_channel_2);
    }
    
    char state_msg[256];
    sprintf(state_msg, 
            "=== Motor Pin State ===\r\n"
            "GPIO: IN1=%d, IN2=%d\r\n"
            "PWM: CH1=%d, CH2=%d\r\n"
            "Direction: %d\r\n"
            "Speed: %d%%\r\n"
            "PWM Duty: %d\r\n"
            "Moving: %d\r\n"
            "Emergency: %d\r\n"
            "Position: %.1fmm (%.1f%%)\r\n",
            pin1_state, pin2_state,
            pwm1_value, pwm2_value,
            motor->direction,
            motor->speed_percent,
            motor->pwm_duty,
            motor->is_moving,
            motor->emergency_stop,
            motor->position_mm,
            motor->position_percent);
    
    DEBUG_PRINT("%s", state_msg);
}
