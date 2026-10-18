#include "motor_control.h"
#include "can_protocol.h"
#include "system_state.h"
#include <math.h>

// 电机控制器实例
MotorController motor_x, motor_y, motor_z;

// PID初始化
void pid_init(PIDController *pid, float kp, float ki, float kd) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral = 0;
    pid->prev_error = 0;
}

// PID计算
float pid_compute(PIDController *pid, float setpoint, float actual) {
    float error = setpoint - actual;
    
    // 积分项 (抗饱和)
    pid->integral += error;
    if(pid->integral > PID_MAX_INTEGRAL) pid->integral = PID_MAX_INTEGRAL;
    else if(pid->integral < -PID_MAX_INTEGRAL) pid->integral = -PID_MAX_INTEGRAL;
    
    // 微分项
    float derivative = error - pid->prev_error;
    pid->prev_error = error;
    
    // PID输出
    return (pid->kp * error) + (pid->ki * pid->integral) + (pid->kd * derivative);
}

// 电机初始化
void motor_init(MotorController* motor, uint8_t id, float min_angle, float max_angle) {
    motor->id = id;
    motor->current_angle = 0;
    motor->target_angle = 0;
    motor->min_angle = min_angle;
    motor->max_angle = max_angle;
    motor->last_update = 0;
    
    // 初始化PID控制器
    pid_init(&motor->pid, 0.8f, 0.01f, 0.05f);
    
    printf("Motor %d initialized. Range: %.1f to %.1f degrees\n", 
           id, min_angle, max_angle);
}

// 设置目标角度
void set_target_angle(MotorController* motor, float angle) {
    // 角度限幅
    if(angle > motor->max_angle) angle = motor->max_angle;
    if(angle < motor->min_angle) angle = motor->min_angle;
    
    motor->target_angle = angle;
}

// 电机控制更新
void motor_control_update(MotorController* motor) {
    // 如果系统未运行，不更新控制
    if(g_system.current_state != SYS_STATE_RUNNING) {
        return;
    }
    
    // 获取当前角度（实际应用中从编码器或CAN获取）
    // 这里简化为理想情况
    float actual_angle = motor->current_angle;
    
    // PID计算
    float output = pid_compute(&motor->pid, motor->target_angle, actual_angle);
    
    // 输出限幅
    if(output > MAX_TORQUE) output = MAX_TORQUE;
    else if(output < -MAX_TORQUE) output = -MAX_TORQUE;
    
    // 发送控制命令（MIT模式）
    send_motor_command(motor->id, output);
    
    // 更新当前角度（模拟）
    motor->current_angle += output * 0.001f; // 1ms周期
}

// 紧急停止
void emergency_stop(void) {
    send_motor_command(MOTOR_X, 0);
    send_motor_command(MOTOR_Y, 0);
    send_motor_command(MOTOR_Z, 0);
}
