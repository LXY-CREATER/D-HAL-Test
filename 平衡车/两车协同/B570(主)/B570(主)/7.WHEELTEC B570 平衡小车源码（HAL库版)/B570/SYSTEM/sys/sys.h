#ifndef __SYS_H
#define __SYS_H	
#include "stm32f1xx.h"

//����һЩ���õ��������Ͷ̹ؼ��� 
typedef int32_t  s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef const int32_t sc32;  
typedef const int16_t sc16;  
typedef const int8_t sc8;  

typedef __IO int32_t  vs32;
typedef __IO int16_t  vs16;
typedef __IO int8_t   vs8;

typedef __I int32_t vsc32;  
typedef __I int16_t vsc16; 
typedef __I int8_t vsc8;   

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef const uint32_t uc32;  
typedef const uint16_t uc16;  
typedef const uint8_t uc8; 

typedef __IO uint32_t  vu32;
typedef __IO uint16_t vu16;
typedef __IO uint8_t  vu8;

typedef __I uint32_t vuc32;  
typedef __I uint16_t vuc16; 
typedef __I uint8_t vuc8;  		

//λ������,ʵ��51���Ƶ�GPIO���ƹ���
//����ʵ��˼��,�ο�<<CM3Ȩ��ָ��>>������(87ҳ~92ҳ).
//IO�ڲ����궨��
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
//IO�ڵ�ַӳ��
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 
//IO�ڲ���,ֻ�Ե�һ��IO��!
//ȷ��n��ֵС��16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //���?
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //���� 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //���?
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //���� 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //���?
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //���� 

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //���?
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //���� 

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //���?
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //����

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //���?
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //����

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //���?
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //����

void JTAG_Set(u8 mode);

////JTAGģʽ���ö���
#define JTAG_SWD_DISABLE   0X02
#define SWD_ENABLE         0X01
#define JTAG_SWD_ENABLE    0X00	
extern u8 Way_Angle;                                //��ȡ�Ƕȵ��㷨��1����Ԫ��  2��������  3�������˲� 
extern u8 Flag_front,Flag_back,Flag_Left,Flag_Right,Flag_velocity; //����ң����صı���?
extern u8 Flag_Stop,Flag_Show;                      //���ֹͣ��־λ����ʾ��־�? Ĭ��ֹͣ ��ʾ��
extern int Motor_Left,Motor_Right;                  //���PWM���� Ӧ��Motor�� ��Moto�¾�	
extern int Temperature;                             //�¶ȱ���
extern int Voltage,Middle_angle;                                 //��ص�ѹ������صı���
extern u8 Mode,CCD_Zhongzhi,CCD_Yuzhi,Lidar_Detect;								                //ģʽѡ��Ĭ������ͨ�Ŀ���ģʽ
extern u16 ADV[128]; 
extern u16 determine;                              //�״����ģʽ��һ����־�?
extern float Move_X,Move_Z;
extern u8 LD_Successful_Receive_flag;              //�״�ɹ��������ݱ�־�?
extern float Angle_Balance,Gyro_Balance,Gyro_Turn;  //ƽ�����?ƽ�������� ת��������
extern u32 Distance;                                //���������?
extern u8 PID_Send; 					                      //������ر���?
extern u8 Flash_Send;
extern u8 Flag_follow,Flag_avoid;						      	//���������桢���������ϱ�־λ
extern float Acceleration_Z;                        //Z����ٶȼ�? 
extern volatile u8 delay_flag,delay_50;             //�ṩ��ʱ�ı���
extern float Balance_Kp,Balance_Kd,Velocity_Kp,Velocity_Ki,Turn_Kp,Turn_Kd;//PID参数均放大100倍
extern float Yaw_Kp,Yaw_Kd;                          //航向保持PD参数
extern int   Yaw_Limit;                               //航向输出限幅
extern u8 Cooperative_Mode_Flag;                    //协同搬运模式使能标志
extern u8 Last_Sent_Cmd;                            //上次发送给从机的命令
extern u8 Slave_Turn_Left,Slave_Turn_Right;         //从机转向标志//�ϴη��͸��ӻ�������
//////////////////////////////////////////////////////////////////////////////
//����Ϊ��ຯ��?
void WFI_SET(void);		  //ִ��WFIָ��
void INTX_DISABLE(void);//�ر������ж�
void INTX_ENABLE(void);	//���������ж�
void MSR_MSP(u32 addr);	//���ö�ջ��ַ

#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "delay.h"
#include "control.h"
#include "DataScope_DP.h"
#include "filter.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "mpu6050.h"
#include "show.h"
#include "encoder.h"
#include "ioi2c.h"
#include "key.h"
#include "led.h"
#include "oled.h"
#include "usart3.h"
#include "dmpKey.h"
#include "dmpmap.h"
#include <math.h>
#include <string.h> 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "Lidar.h"
#include "ELE_CCD.h"
#endif
