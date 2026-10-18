#ifndef __KALMAN_YAW_H
#define __KALMAN_YAW_H

#include "sys.h"

/*
 * 航向卡尔曼滤波器
 * 融合 IMU(Yaw) 和 两轮编码器 得到更稳定的航向角
 *
 * 预测: 用编码器差速推算的偏航率
 * 更新: 用 IMU DMP 输出的 Yaw 角
 */

typedef struct {
    float yaw;   /* 融合后的航向角 (deg) */
    float P;     /* 误差协方差          */
    float Q;     /* 过程噪声            */
    float R;     /* 测量噪声            */
} Kalman_Yaw_t;

void  Kalman_Yaw_Init(Kalman_Yaw_t *kf, float init_yaw, float Q, float R);
float Kalman_Yaw_Update(Kalman_Yaw_t *kf, float yaw_imu, float yaw_rate_enc, float dt);

#endif
