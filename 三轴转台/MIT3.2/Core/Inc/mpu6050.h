#ifndef __MPU6050_H
#define __MPU6050_H
#define GYRO_SCALE  (1.0f / 16.4f)   
#define ACC_SCALE   (1.0f / 4096.0f) 
#include <stdint.h>
#include "i2c.h"
#include "math.h"
extern float pitch; 
extern float roll;
extern float yaw;
void MPU6050_Init(void);
void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ,
                     int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ); 
void Calibrate_Gyro(void);


uint8_t MPU6050_GetID(void);
void computer(void);

#endif
