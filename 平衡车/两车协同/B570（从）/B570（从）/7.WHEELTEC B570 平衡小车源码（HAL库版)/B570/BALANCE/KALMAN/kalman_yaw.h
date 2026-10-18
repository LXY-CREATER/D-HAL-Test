#ifndef __KALMAN_YAW_H
#define __KALMAN_YAW_H

#include "sys.h"

typedef struct {
    float yaw;
    float P;
    float Q;
    float R;
} Kalman_Yaw_t;

void  Kalman_Yaw_Init(Kalman_Yaw_t *kf, float init_yaw, float Q, float R);
float Kalman_Yaw_Update(Kalman_Yaw_t *kf, float yaw_imu, float yaw_rate_enc, float dt);

#endif
