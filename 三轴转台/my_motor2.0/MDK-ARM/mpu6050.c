#include "MPU6050.h"
#include "i2c.h"
 #include "stm32f1xx_hal.h"

#define MPU6050_TIMEOUT     100           // ³¬Ê±Ê±¼ä ms
// ????I2C??
 float pitch=0.0f; 
 float roll=0.0f;
 float  yaw=0.0f;
static float my_dt = 0.01f;   
static float gx_bias = 0;
static float gy_bias = 0;
static float gz_bias = 0;
static int16_t ax; 
static int16_t ay;
static int16_t az;
static int16_t gx;
static int16_t gy;
static int16_t gz;
static float axg;
static float ayg;
static float azg;
static float gx_dps;
static float gy_dps;
static float gz_dps;
static float roll_acc;
static float pitch_acc;
#define RAD_TO_DEG 57.2957795131f
// ???????
#define MPU6050_ADDRESS         0xD0
#define MPU6050_PWR_MGMT_1      0x6B
#define MPU6050_PWR_MGMT_2      0x6C
#define MPU6050_SMPLRT_DIV      0x19
#define MPU6050_CONFIG          0x1A
#define MPU6050_GYRO_CONFIG     0x1B
#define MPU6050_ACCEL_CONFIG    0x1C
#define MPU6050_WHO_AM_I        0x75
#define MPU6050_ACCEL_XOUT_H    0x3B
#define MPU6050_ACCEL_XOUT_L    0x3C
#define MPU6050_ACCEL_YOUT_H    0x3D
#define MPU6050_ACCEL_YOUT_L    0x3E
#define MPU6050_ACCEL_ZOUT_H    0x3F
#define MPU6050_ACCEL_ZOUT_L    0x40
#define MPU6050_GYRO_XOUT_H     0x43
#define MPU6050_GYRO_XOUT_L     0x44
#define MPU6050_GYRO_YOUT_H     0x45
#define MPU6050_GYRO_YOUT_L     0x46
#define MPU6050_GYRO_ZOUT_H     0x47
#define MPU6050_GYRO_ZOUT_L     0x48

static void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data) {
    HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDRESS, RegAddress, 
                      I2C_MEMADD_SIZE_8BIT, &Data, 1, 100);
}

static uint8_t MPU6050_ReadReg(uint8_t RegAddress) {
    uint8_t data;
    HAL_I2C_Mem_Read(&hi2c2, MPU6050_ADDRESS, RegAddress,
                     I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    return data;
}


void MPU6050_Init(void)
{
    /* 1. µÈ´ý MPU6050 ÉÏµç¾ÍÐ÷£¨¿ÉÑ¡£© */
    HAL_Delay(100);

    /* 2. Ð´¼Ä´æÆ÷ ¡ª¡ª ÓÃ HAL_I2C_Mem_Write() */
    HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDRESS, MPU6050_PWR_MGMT_1,
                      I2C_MEMADD_SIZE_8BIT, (uint8_t[]){0x01}, 1, MPU6050_TIMEOUT);

    HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDRESS, MPU6050_PWR_MGMT_2,
                      I2C_MEMADD_SIZE_8BIT, (uint8_t[]){0x00}, 1, MPU6050_TIMEOUT);

    HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDRESS, MPU6050_SMPLRT_DIV,
                      I2C_MEMADD_SIZE_8BIT, (uint8_t[]){0x09}, 1, MPU6050_TIMEOUT);

    HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDRESS, MPU6050_CONFIG,
                      I2C_MEMADD_SIZE_8BIT, (uint8_t[]){0x06}, 1, MPU6050_TIMEOUT);

    HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDRESS, MPU6050_GYRO_CONFIG,
                      I2C_MEMADD_SIZE_8BIT, (uint8_t[]){0x18}, 1, MPU6050_TIMEOUT);

    HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDRESS, MPU6050_ACCEL_CONFIG,
                      I2C_MEMADD_SIZE_8BIT, (uint8_t[]){0x18}, 1, MPU6050_TIMEOUT);
}
	//    MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x00);
//    HAL_Delay(100);
//    
//    // ????????
//    MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x07);   // 125Hz???
//    
//    // ???????
//    MPU6050_WriteReg(MPU6050_CONFIG, 0x06);       // 5Hz??
//    
//    // ??????? ±2000°/s
//    MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);
//    
//    // ???????? ±8g
//    MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x10);
//    
//    HAL_Delay(100);


uint8_t MPU6050_GetID(void) {
    return MPU6050_ReadReg(MPU6050_WHO_AM_I);
}

//void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ,
//                     int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ) 
//{
//    uint8_t data[14];
//    HAL_I2C_Mem_Read(&hi2c2, MPU6050_ADDRESS, MPU6050_ACCEL_XOUT_H,
//                     I2C_MEMADD_SIZE_8BIT, data, 14, 100);
//    
//    *AccX = (int16_t)((data[0] << 8) | data[1]);
//    *AccY = (int16_t)((data[2] << 8) | data[3]);
//    *AccZ = (int16_t)((data[4] << 8) | data[5]);
//    *GyroX = (int16_t)((data[8] << 8) | data[9]);
//    *GyroY = (int16_t)((data[10] << 8) | data[11]);
//    *GyroZ = (int16_t)((data[12] << 8) | data[13]);

void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
						int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
	uint8_t DataH, DataL;								//¶¨ÒåÊý¾Ý¸ß8Î»ºÍµÍ8Î»µÄ±äÁ¿
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_H);		//¶ÁÈ¡¼ÓËÙ¶È¼ÆXÖáµÄ¸ß8Î»Êý¾Ý
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_L);		//¶ÁÈ¡¼ÓËÙ¶È¼ÆXÖáµÄµÍ8Î»Êý¾Ý
	*AccX = (DataH << 8) | DataL;						//Êý¾ÝÆ´½Ó£¬Í¨¹ýÊä³ö²ÎÊý·µ»Ø
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_H);		//¶ÁÈ¡¼ÓËÙ¶È¼ÆYÖáµÄ¸ß8Î»Êý¾Ý
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_L);		//¶ÁÈ¡¼ÓËÙ¶È¼ÆYÖáµÄµÍ8Î»Êý¾Ý
	*AccY = (DataH << 8) | DataL;						//Êý¾ÝÆ´½Ó£¬Í¨¹ýÊä³ö²ÎÊý·µ»Ø
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_H);		//¶ÁÈ¡¼ÓËÙ¶È¼ÆZÖáµÄ¸ß8Î»Êý¾Ý
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_L);		//¶ÁÈ¡¼ÓËÙ¶È¼ÆZÖáµÄµÍ8Î»Êý¾Ý
	*AccZ = (DataH << 8) | DataL;						//Êý¾ÝÆ´½Ó£¬Í¨¹ýÊä³ö²ÎÊý·µ»Ø
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_XOUT_H);		//¶ÁÈ¡ÍÓÂÝÒÇXÖáµÄ¸ß8Î»Êý¾Ý
	DataL = MPU6050_ReadReg(MPU6050_GYRO_XOUT_L);		//¶ÁÈ¡ÍÓÂÝÒÇXÖáµÄµÍ8Î»Êý¾Ý
	*GyroX = (DataH << 8) | DataL;						//Êý¾ÝÆ´½Ó£¬Í¨¹ýÊä³ö²ÎÊý·µ»Ø
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_YOUT_H);		//¶ÁÈ¡ÍÓÂÝÒÇYÖáµÄ¸ß8Î»Êý¾Ý
	DataL = MPU6050_ReadReg(MPU6050_GYRO_YOUT_L);		//¶ÁÈ¡ÍÓÂÝÒÇYÖáµÄµÍ8Î»Êý¾Ý
	*GyroY = (DataH << 8) | DataL;						//Êý¾ÝÆ´½Ó£¬Í¨¹ýÊä³ö²ÎÊý·µ»Ø
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_H);		//¶ÁÈ¡ÍÓÂÝÒÇZÖáµÄ¸ß8Î»Êý¾Ý
	DataL = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_L);		//¶ÁÈ¡ÍÓÂÝÒÇZÖáµÄµÍ8Î»Êý¾Ý
	*GyroZ = (DataH << 8) | DataL;						//Êý¾ÝÆ´½Ó£¬Í¨¹ýÊä³ö²ÎÊý·µ»Ø
}

void Calibrate_Gyro()
{
    int32_t sum[3] = {0};
    int i;
    for (i = 0; i < 200; i++)  {
        
        MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);
        sum[0] += gx;
        sum[1] += gy;
        sum[2] += gz;
        HAL_Delay(5);
    }
    gx_bias = sum[0] / 200.0f;
    gy_bias = sum[1] / 200.0f;
    gz_bias = sum[2] / 200.0f;
}
//void computer()
//{
//	uint32_t start = HAL_GetTick();
//	MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);

//    axg = ax * ACC_SCALE;
//    ayg = ay * ACC_SCALE;
//    azg = az * ACC_SCALE;
//    
//    gx_dps = (gx - gx_bias) * GYRO_SCALE;
//    gy_dps = (gy - gy_bias) * GYRO_SCALE;
//    gz_dps = (gz - gz_bias) * GYRO_SCALE;

//    roll_acc = atan2f(ayg, azg) * RAD_TO_DEG;
//    pitch_acc = atan2f(-axg, sqrtf(ayg*ayg + azg*azg)) * RAD_TO_DEG;

//    roll = 0.98f * (roll + gx_dps * my_dt) + 0.02f * roll_acc;
//    pitch = 0.98f * (pitch + gy_dps * my_dt) + 0.02f * pitch_acc;
//    yaw += gz_dps * my_dt;

//    roll = fmodf(roll + 180, 360) - 180;
//    pitch = fmodf(pitch + 180, 360) - 180;
//    yaw = fmodf(yaw + 180, 360) - 180;
//		
//		uint32_t elapsed = HAL_GetTick() - start;
//    my_dt = elapsed / 1000.0f;
//}
