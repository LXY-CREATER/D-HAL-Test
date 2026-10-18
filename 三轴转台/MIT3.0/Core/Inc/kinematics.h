#ifndef __KINEMATICS_H__
#define __KINEMATICS_H__

#include "config.h"

// º¯ÊýÉùÃ÷
void Kinematic_Solve(float vx, float vy, float omega, float* roll, float* pitch, float* yaw);
void SetMotorTargets(float roll, float pitch, float yaw);

#endif /* __KINEMATICS_H__ */
