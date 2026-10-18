#ifndef __MOTOR_CONTROL_H
#define __MOTOR_CONTROL_H

#include "stm32f1xx_hal.h"

// 电机ID定义
#define MOTOR_X 0
#define MOTOR_Y 1
#define MOTOR_Z 2

// PID控制器参数
#define PID_MAX_INTEGRAL 1000.0f
#define MAX_TORQUE 5.0f // 最大扭矩5Nm

// PID控制器结构
typedef struct {
    float kp, ki, kd;
    float integral;
    float prev_error;
} PIDController;

// 电机控制器结构
typedef struct {
    uint8_t id;
    float current_angle;
    float target_angle;
    float min_angle;
    float max_angle;
    uint32_t last_update;
    PIDController pid;
} MotorController;

// 全局电机实例
extern MotorController motor_x, motor_y, motor_z;

// 函数声明
void motor_init(MotorController* motor, uint8_t id, float min_angle, float max_angle);
void set_target_angle(MotorController* motor, float angle);
void motor_control_update(MotorController* motor);
void emergency_stop(void);
void pid_init(PIDController *pid, float kp, float ki, float kd);
float pid_compute(PIDController *pid, float setpoint, float actual);

#endif
