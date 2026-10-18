#ifndef __PID_CONTROLLER_H
#define __PID_CONTROLLER_H

typedef struct {
  float kp;
  float ki;
  float kd;
  float integral;
  float prev_error;
  float max_output;
} PIDController;

void pid_init(PIDController* pid, float kp, float ki, float kd);
float pid_compute(PIDController* pid, float setpoint, float measurement);

#endif
