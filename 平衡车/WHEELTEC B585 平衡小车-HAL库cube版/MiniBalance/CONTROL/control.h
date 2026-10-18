/***********************************************
��˾����Ȥ�Ƽ�(��ݸ)���޹�˾
Ʒ�ƣ�WHEELTEC
������wheeltec.net
�Ա����̣�shop114407458.taobao.com 
����ͨ: https://minibalance.aliexpress.com/store/4455017
�汾��V1.0
�޸�ʱ�䣺2022-09-05

Brand: WHEELTEC
Website: wheeltec.net
Taobao shop: shop114407458.taobao.com 
Aliexpress: https://minibalance.aliexpress.com/store/4455017
Version: V1.0
Update��2022-09-05

All rights reserved
***********************************************/
#ifndef __CONTROL_H
#define __CONTROL_H
#include "sys.h"

#define PI 3.14159265							//PIԲ����
#define Control_Frequency  200.0	//��������ȡƵ��
#define Diameter_67  67.0 				//����ֱ��67mm 
#define EncoderMultiples   4.0 		//��������Ƶ��
#define Encoder_precision  500.0 	//���������� 500��
#define Reduction_Ratio  30.0			//���ٱ�30
#define Perimeter  210.4867 			//�ܳ�����λmm


#define DIFFERENCE 100
#define INT PBin(9)   //PB9���ӵ�MPU6050���ж�����


//С����ģʽ����
#define Normal_Mode							0
#define Lidar_Avoid_Mode					1
#define Lidar_Follow_Mode					2
#define Lidar_Straight_Mode                 3
#define ELE_Line_Patrol_Mode				4
#define CCD_Line_Patrol_Mode				5
#define ROS_Mode				            6
#define Ultrasonic_Avoid_Mode               7
#define Ultrasonic_Follow_Mode              8
#define Bluetooth_Mode                      9

//����ģʽ�Ĳ���
#define  avoid_Distance 300//���Ͼ���300mm
#define avoid_Angle1 50 //���ϵĽǶȣ���310~360��0~50��ķ�Χ
#define avoid_Angle2 310
#define avoid_speed 200    //�����ٶ�
#define turn_speed 800;    //����ת���ٶ�

//�״���ֱ�ߵĲ���
#define Initial_speed 200//С���ĳ�ʼ�ٶȴ��Ϊ200mmÿ��
#define Limit_time 500   //����ʱ�䣬5ms�ж�*��ֵ=ʱ�� ���������3s
#define refer_angle1  71 //������ĽǶ�1
#define refer_angle2  74 //������ĽǶ�2

int Balance(float angle,float gyro);
int Velocity(int encoder_left,int encoder_right);
int Turn(float gyro);
void Set_Pwm(int motor_left,int motor_right);
void Limit_Pwm(void);
int PWM_Limit(int IN,int max,int min);
u8 Turn_Off(float angle, int voltage);
void Middle_angle_Check(void);
void Get_Angle(u8 way);
int myabs(int a);
int Pick_Up(float Acceleration,float Angle,int encoder_left,int encoder_right);
int Put_Down(float Angle,int encoder_left,int encoder_right);
void Get_Velocity_Form_Encoder(int encoder_left,int encoder_right);
void Choose(int encoder_left,int encoder_right);
void Read_distance(void);
void Lidar_Avoid(void);
void Lidar_Follow(void);
void Lidar_Straight(void);
void Read_Distane(void);
void Lidar_Avoid(void);
void Select_Zhongzhi(void);
void Normal(void);
void Remote_Control(void);
void Bluetooth_Sequence(void);
extern short Accel_Y,Accel_Z,Accel_X,Accel_Angle_x,Accel_Angle_y,Gyro_X,Gyro_Z,Gyro_Y;
extern float Yaw;  // MPU6050 DMP Z-axis heading angle (0-360 deg)

#endif

