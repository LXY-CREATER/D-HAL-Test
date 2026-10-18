#include "motor_control.h"
#include "can_protocol.h"
#include "system_state.h"

// 电机控制器实例
MotorController motor_x, motor_y, motor_z;

void motor_init(MotorController* motor, uint8_t id, float min_angle, float max_angle) {
  motor->id = id;
  motor->current_angle = 0;
  motor->target_angle = 0;
  motor->min_angle = min_angle;
  motor->max_angle = max_angle;
  pid_init(&motor->pid, 0.8, 0.01, 0.05); // PID参数需根据实际调整
}

void set_target_angle(MotorController* motor, float angle) {
  // 角度限幅
  if(angle > motor->max_angle) angle = motor->max_angle;
  if(angle < motor->min_angle) angle = motor->min_angle;
  
  motor->target_angle = angle;
}

void motor_control_update(MotorController* motor) {
  
	// 紧急停止时不更新控制
  if(g_system.current_state != SYS_STATE_RUNNING) {
    return;
  }
	
	// 获取当前角度（实际应用中从编码器或CAN获取）
  // 这里简化为理想情况
  float actual_angle = motor->current_angle;
  
  // PID计算
  float output = pid_compute(&motor->pid, motor->target_angle, actual_angle);
  
  // 发送控制命令（MIT模式）
  send_motor_command(motor->id, output);
  
  // 更新当前角度（模拟）
  motor->current_angle += output * 0.01; // 1ms周期
}

void emergency_stop(void) {
  send_motor_command(MOTOR_X, 0);
  send_motor_command(MOTOR_Y, 0);
  send_motor_command(MOTOR_Z, 0);
}
