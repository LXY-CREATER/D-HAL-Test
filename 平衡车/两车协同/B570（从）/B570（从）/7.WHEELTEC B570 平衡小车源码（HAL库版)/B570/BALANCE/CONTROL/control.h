/***********************************************
��˾����Ȥ�Ƽ�����ݸ�����޹�˾
Ʒ�ƣ�WHEELTEC
������wheeltec.net
�Ա����̣�shop114407458.taobao.com 
����ͨ: https://minibalance.aliexpress.com/store/4455017
�汾��5.7
�޸�ʱ�䣺2021-04-29

Brand: WHEELTEC
Website: wheeltec.net
Taobao shop: shop114407458.taobao.com 
Aliexpress: https://minibalance.aliexpress.com/store/4455017
Version:5.7
Update��2021-04-29

All rights reserved
***********************************************/
#ifndef __CONTROL_H
#define __CONTROL_H
#include "sys.h"
extern int Sensor_Left,Sensor_Middle,Sensor_Right,Sensor;

#define PI 3.14159265							//PIԲ����
#define Control_Frequency  200.0	//��������ȡƵ��
#define Diameter_67  67.0 				//����ֱ��67mm 
#define EncoderMultiples   4.0 		//��������Ƶ��
#define Encoder_precision  13.0 	//���������� 13��
#define Reduction_Ratio  30.0			//���ٱ�30
#define Perimeter  210.4867 			//周长，单位mm
#define WHEEL_BASE  150.0f              //轴距 mm (B570履带底盘)

//小车模式定义
#define Normal_Mode							0
#define Ultrasonic_Avoid_Mode               1
#define Ultrasonic_Follow_Mode              2
#define Lidar_Avoid_Mode					3
#define Lidar_Follow_Mode					4
#define Lidar_Straight_Mode                 5
#define ELE_Line_Patrol_Mode				7
#define CCD_Line_Patrol_Mode				6
#define Cooperative_Mode                   8  //协同搬运模式

//����ģʽ�Ĳ���
#define  avoid_Distance 350//���Ͼ���300mm
#define avoid_Angle1 50 //���ϵĽǶȣ���310~360��0~50��ķ�Χ
#define avoid_Angle2 310
#define avoid_speed 30    //�����ٶ�
#define turn_speed 1000;    //����ת���ٶ�

//�״���ֱ�ߵĲ���
#define Initial_speed 30//С���ĳ�ʼ�ٶȴ��Ϊ200mmÿ��
#define Limit_time 500   //����ʱ�䣬5ms�ж�*��ֵ=ʱ�� ���������3s
#define refer_angle1  71 //������ĽǶ�1
#define refer_angle2  74 //������ĽǶ�2

//�״�������
#define Follow_distance 1500  //�״����ģʽ��Զ����

#define Barrier_Detected						1
#define No_Barrier								0
#define tracking_speed 40      //��С��һ�����300mm/s���ٶ�
#define Detect_distance 700//������Ϊ700mm


#define DIFFERENCE 100
#define INT PAin(12)   //PA12���ӵ�MPU6050���ж�����

int Balance(float angle,float gyro);
int Velocity(int encoder_left,int encoder_right);
int Turn(float gyro);
void Set_Pwm(int motor_left,int motor_right);
void Key(void);
void Limit_Pwm(void);
int PWM_Limit(int IN,int max,int min);
u8 Turn_Off(float angle, int voltage);
void Get_Angle(u8 way);
int myabs(int a);
int Pick_Up(float Acceleration,float Angle,int encoder_left,int encoder_right);
int Put_Down(float Angle,int encoder_left,int encoder_right);
void Get_Velocity_Form_Encoder(int encoder_left,int encoder_right);
u8 Choose(void);
void Lidar_Avoid(void);
void Lidar_Straight(void);
void Lidar_Follow(void);
void CCD_Mode(void);
int CCD_turn(u8 CCD,float gyro);//ת�����
int ELE_turn(float gyro);//ת�����
void ELE_Mode(void);
void Select_Zhongzhi(void);
float Keep(void *kf, float *target_yaw, u8 *locked, float *gyro_lpf, float Q, float R);
#endif
