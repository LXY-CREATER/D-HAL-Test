#include "pid_controller.h"

void pid_init(PIDController* pid, float kp, float ki, float kd) {
  pid->kp = kp;
  pid->ki = ki;
  pid->kd = kd;
  pid->integral = 0;
  pid->prev_error = 0;
  pid->max_output = 1000; // 输出限幅
}

float pid_compute(PIDController* pid, float setpoint, float measurement) {
  float error = setpoint - measurement;
  
  // 积分项
  pid->integral += error;
  
  // 微分项
  float derivative = error - pid->prev_error;
  pid->prev_error = error;
  
  // PID计算
  float output = pid->kp * error + 
                pid->ki * pid->integral + 
                pid->kd * derivative;
  
  // 输出限幅
  if(output > pid->max_output) output = pid->max_output;
  if(output < -pid->max_output) output = -pid->max_output;
  
  return output;
}
