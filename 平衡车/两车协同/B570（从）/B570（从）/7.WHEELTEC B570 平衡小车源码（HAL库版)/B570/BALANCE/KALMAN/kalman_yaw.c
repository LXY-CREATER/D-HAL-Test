/**
 ******************************************************************************
 * @file    kalman_yaw.c
 * @brief   航向卡尔曼滤波器 — 融合 IMU Yaw 和编码器差速
 ******************************************************************************
 */

#include "kalman_yaw.h"

void Kalman_Yaw_Init(Kalman_Yaw_t *kf, float init_yaw, float Q, float R)
{
    kf->yaw = init_yaw;
    kf->P   = 1.0f;
    kf->Q   = Q;
    kf->R   = R;
}

float Kalman_Yaw_Update(Kalman_Yaw_t *kf, float yaw_imu, float yaw_rate_enc, float dt)
{
    float yaw_pred, P_pred, K;
    float yaw_err;

    yaw_pred = kf->yaw + yaw_rate_enc * dt;
    P_pred   = kf->P + kf->Q;

    yaw_err = yaw_imu - yaw_pred;
    if (yaw_err > 180.0f)  yaw_err -= 360.0f;
    if (yaw_err < -180.0f) yaw_err += 360.0f;

    K = P_pred / (P_pred + kf->R);
    kf->yaw = yaw_pred + K * yaw_err;

    if (kf->yaw > 180.0f)  kf->yaw -= 360.0f;
    if (kf->yaw < -180.0f) kf->yaw += 360.0f;

    kf->P = (1.0f - K) * P_pred;
    if (kf->P < 0.001f) kf->P = 0.001f;

    return kf->yaw;
}
