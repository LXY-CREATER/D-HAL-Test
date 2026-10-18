#include "kinematics.h"
#include "config.h"

// 运动学解算
void Kinematic_Solve(float vx, float vy, float omega, float* roll, float* pitch, float* yaw) {
    static float yaw_integral = 0.0f;
    const float dt = 1.0f / CONTROL_FREQ;
    const float rad_to_deg=180.0f/M_PI;//弧度转角度系数
	
    // 横滚角计算 (X轴)
    *roll = atan2f(vy, vx) * rad_to_deg;
    
    // 俯仰角计算 (Y轴)
    float speed = sqrtf(vx*vx + vy*vy);
    *pitch = asinf(vx / (speed + 1e-5f)) * rad_to_deg;  // 避免除零
    
    // 偏航角积分 (Z轴)
    yaw_integral += omega * dt;
    *yaw = yaw_integral * rad_to_deg;
    
    // 角度限幅 [-180°, 180°]
    *yaw = fmodf(*yaw + 180.0f, 360.0f);
    if (*yaw < 0) *yaw += 360.0f;
    *yaw -= 180.0f;
    
    // 安全限幅
    *roll = fmaxf(fminf(*roll, MAX_ROLL_ANGLE), -MAX_ROLL_ANGLE);
    *pitch = fmaxf(fminf(*pitch, MAX_PITCH_ANGLE), -MAX_PITCH_ANGLE);
    *yaw = fmaxf(fminf(*yaw, MAX_YAW_ANGLE), -MAX_YAW_ANGLE);
}

// 设置电机目标位置
void SetMotorTargets(float roll, float pitch, float yaw) {
    Motors[0].target_position = roll;    // 主轴 - 横滚
    Motors[1].target_position = pitch;   // 从轴1 - 俯仰
    Motors[2].target_position = yaw;     // 从轴2 - 偏航
}
