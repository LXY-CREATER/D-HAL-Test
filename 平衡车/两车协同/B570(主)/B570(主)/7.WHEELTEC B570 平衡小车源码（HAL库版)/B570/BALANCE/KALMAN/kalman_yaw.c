/**
 ******************************************************************************
 * @file    kalman_yaw.c
 * @brief   航向卡尔曼滤波器 — 融合 IMU Yaw 和编码器差速
 *
 * 原理:
 *   编码器差速 -> 偏航角速度(短时稳定,长时累积误差)
 *   IMU Yaw  -> 绝对航向(长时参考,短时有噪声)
 *   卡尔曼滤波将两者最优融合
 ******************************************************************************
 */

#include "kalman_yaw.h"

/**
 * @brief  初始化航向卡尔曼滤波器
 * @param  kf       : 滤波器结构体指针
 * @param  init_yaw : 初始航向角 (deg)，传当前 IMU Yaw 以快速收敛
 * @param  Q        : 过程噪声协方差 (编码器偏航率的置信度, 越大越相信预测)
 * @param  R        : 测量噪声协方差 (IMU Yaw的置信度, 越大越不相信测量)
 */
void Kalman_Yaw_Init(Kalman_Yaw_t *kf, float init_yaw, float Q, float R)
{
    kf->yaw = init_yaw;  /* 用当前航向初始化，避免起步跳变 */
    kf->P   = 1.0f;      /* 初始协方差 */
    kf->Q   = Q;
    kf->R   = R;
}

/**
 * @brief  卡尔曼滤波一步更新
 * @param  kf          : 滤波器结构体指针
 * @param  yaw_imu     : IMU 测量的 Yaw 角 (deg)
 * @param  yaw_rate_enc: 编码器差速推算的偏航角速度 (deg/s)
 * @param  dt          : 时间步长 (s)
 * @return 融合后的航向角 (deg)
 *
 *   ----- 预测 -----
 *   yaw_pred = yaw + yaw_rate_enc * dt
 *   P_pred   = P + Q
 *
 *   ----- 更新 -----
 *   K       = P_pred / (P_pred + R)
 *   yaw    += K * (yaw_imu - yaw_pred)
 *   P       = (1 - K) * P_pred
 */
float Kalman_Yaw_Update(Kalman_Yaw_t *kf, float yaw_imu, float yaw_rate_enc, float dt)
{
    float yaw_pred, P_pred, K;
    float yaw_err;

    /* ========== 预测 ========== */
    yaw_pred = kf->yaw + yaw_rate_enc * dt;
    P_pred   = kf->P + kf->Q;

    /* ========== 更新 (用IMU Yaw测量) ========== */
    /* 角度差值归一化到 -180~180 */
    yaw_err = yaw_imu - yaw_pred;
    if (yaw_err > 180.0f)  yaw_err -= 360.0f;
    if (yaw_err < -180.0f) yaw_err += 360.0f;

    K = P_pred / (P_pred + kf->R);
    kf->yaw = yaw_pred + K * yaw_err;

    /* 输出归一化 */
    if (kf->yaw > 180.0f)  kf->yaw -= 360.0f;
    if (kf->yaw < -180.0f) kf->yaw += 360.0f;

    kf->P = (1.0f - K) * P_pred;
    if (kf->P < 0.001f) kf->P = 0.001f;  /* 防止过小 */

    return kf->yaw;
}
