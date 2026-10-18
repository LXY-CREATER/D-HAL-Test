#ifndef __MOTOR_CONTROL_H
#define __MOTOR_CONTROL_H

#include "stm32f1xx_hal.h"
#include "pid_controller.h"

// 电机ID定义
#define MOTOR_X 0
#define MOTOR_Y 1
#define MOTOR_Z 2

// 电机控制结构体
typedef struct {
  uint8_t id;
  float current_angle;
  float target_angle;
  float max_angle;
  float min_angle;
  PIDController pid;
  uint32_t last_update;
} MotorController;

// 声明全局变量
extern MotorController motor_x, motor_y, motor_z;


// 初始化电机控制器
void motor_init(MotorController* motor, uint8_t id, float min_angle, float max_angle);

// 设置目标角度
void set_target_angle(MotorController* motor, float angle);

// 更新控制循环
void motor_control_update(MotorController* motor);

// 紧急停止
void emergency_stop(void);

#endif
