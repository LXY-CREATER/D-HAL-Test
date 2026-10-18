#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "main.h" //HAL库文件声明

//LF为左前轮，LB为左后轮，RF为右前轮，RB为右后轮
//根据L298N真值表编写
#define LF_GO 	     HAL_GPIO_WritePin(IN1_GPIO_Port,  IN1_Pin,  GPIO_PIN_SET);     HAL_GPIO_WritePin(IN2_GPIO_Port,  IN2_Pin,  GPIO_PIN_RESET)
#define LB_GO     	 HAL_GPIO_WritePin(IN11_GPIO_Port, IN11_Pin, GPIO_PIN_SET);     HAL_GPIO_WritePin(IN22_GPIO_Port, IN22_Pin, GPIO_PIN_RESET)
#define RF_GO	     HAL_GPIO_WritePin(IN3_GPIO_Port,  IN3_Pin,  GPIO_PIN_SET);     HAL_GPIO_WritePin(IN4_GPIO_Port,  IN4_Pin,  GPIO_PIN_RESET)
#define RB_GO	     HAL_GPIO_WritePin(IN33_GPIO_Port, IN33_Pin, GPIO_PIN_SET);     HAL_GPIO_WritePin(IN44_GPIO_Port, IN44_Pin, GPIO_PIN_RESET)
#define LF_BACK 	   HAL_GPIO_WritePin(IN1_GPIO_Port,  IN1_Pin,  GPIO_PIN_RESET);   HAL_GPIO_WritePin(IN2_GPIO_Port,  IN2_Pin,  GPIO_PIN_SET)
#define LB_BACK	   HAL_GPIO_WritePin(IN11_GPIO_Port, IN11_Pin, GPIO_PIN_RESET);   HAL_GPIO_WritePin(IN22_GPIO_Port, IN22_Pin, GPIO_PIN_SET)
#define RF_BACK		 HAL_GPIO_WritePin(IN3_GPIO_Port,  IN3_Pin,  GPIO_PIN_RESET);   HAL_GPIO_WritePin(IN4_GPIO_Port,  IN4_Pin,  GPIO_PIN_SET)
#define RB_BACK		 HAL_GPIO_WritePin(IN33_GPIO_Port, IN33_Pin, GPIO_PIN_RESET);   HAL_GPIO_WritePin(IN44_GPIO_Port, IN44_Pin, GPIO_PIN_SET)
#define LF_STOP	   HAL_GPIO_WritePin(IN1_GPIO_Port,  IN1_Pin,  GPIO_PIN_RESET);   HAL_GPIO_WritePin(IN2_GPIO_Port,  IN2_Pin,  GPIO_PIN_RESET)
#define LB_STOP	   HAL_GPIO_WritePin(IN11_GPIO_Port, IN11_Pin, GPIO_PIN_RESET);   HAL_GPIO_WritePin(IN22_GPIO_Port, IN22_Pin, GPIO_PIN_RESET)
#define RF_STOP		 HAL_GPIO_WritePin(IN3_GPIO_Port,  IN3_Pin,  GPIO_PIN_RESET);   HAL_GPIO_WritePin(IN4_GPIO_Port,  IN4_Pin,  GPIO_PIN_RESET)
#define RB_STOP     HAL_GPIO_WritePin(IN33_GPIO_Port, IN33_Pin, GPIO_PIN_RESET);   HAL_GPIO_WritePin(IN44_GPIO_Port, IN44_Pin, GPIO_PIN_RESET)

void L_MOTOR_GO(void);         //左侧两个轮子向前转

void R_MOTOR_GO(void);         //右侧两个轮子向前转

void L_MOTOR_BACK(void);       //左侧两个轮子向后转

void R_MOTOR_BACK(void);       //右侧两个轮子向后转

void L_MOTOR_STOP(void);       //左侧两个轮子停转

void R_MOTOR_STOP(void);       //右侧两个轮子停转

#endif 
