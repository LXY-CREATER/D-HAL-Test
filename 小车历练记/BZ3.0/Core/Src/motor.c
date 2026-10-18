#include "motor.h"

//LF为左前轮，LB为左后轮，RF为右前轮，RB为右后轮

void L_MOTOR_GO(void)    //左侧两个轮子向前转
{

	LF_GO;
	LB_GO;
}

void R_MOTOR_GO(void)    //右侧两个轮子向前转
{
	RF_GO;
	RB_GO;
}

void L_MOTOR_BACK(void)  //左侧两个轮子向后转
{
	LF_BACK ;
	LB_BACK ;
}

void R_MOTOR_BACK(void)  //右侧两个轮子向后转
{
	RF_BACK;
	RB_BACK;
}

void L_MOTOR_STOP(void)  //左侧两个轮子停转
{
	LF_STOP;
	LB_STOP;
}

void R_MOTOR_STOP(void)  //右侧两个轮子停转
{
	RF_STOP;
	RB_STOP;
}
