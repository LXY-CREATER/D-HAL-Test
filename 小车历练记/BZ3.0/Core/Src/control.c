#include "control.h"
#include "motor.h"
extern uint32_t speed;

void CAR_GO(void)  //小车前进
{
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1,speed);  // 这里的speed在main.c里定义为全局变量，假设为500，经计算500/2000= 25%的占空比，即速度为全速的1/4
	L_MOTOR_GO();
	R_MOTOR_GO();
}
void CAR_BACK(void)  //小车后退
{
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1,speed);
	L_MOTOR_BACK();
	R_MOTOR_BACK();
}
void CAR_LGO(void)  //小车原地左拐
{
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1,speed);
	L_MOTOR_BACK();
	R_MOTOR_GO();
}
void CAR_RGO(void)  //小车原地右拐
{
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1,speed);
	L_MOTOR_GO();
	R_MOTOR_BACK();
}
void CAR_STOP(void)  //小车停止
{
	L_MOTOR_STOP();
	R_MOTOR_STOP();
}

//舵机控制
		void SG90_Init()//舵机转90°
	{
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2,1500);  //sg90是逆时针转动
	}
	void SG90_Turn_L()//舵机转135°
	{
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2,2000);
	}
	void SG90_Turn_R()//舵机转45°
	{
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2,1000);
	}

