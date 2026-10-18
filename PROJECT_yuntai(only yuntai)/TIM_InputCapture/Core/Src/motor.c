//// motor.c
//#include "motor.h"
//#include "gpio.h"
//#include "main.h"
//#include "dma.h"
//#include "tim.h"
//#include "usart.h"
//extern TIM_HandleTypeDef htim1;
//extern TIM_HandleTypeDef htim3;

//void Motor_Init(void) 
//	
//		{GPIO_InitTypeDef GPIO_InitStruct = {0};
//    __HAL_RCC_GPIOA_CLK_ENABLE();
//    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5 ;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//		__HAL_RCC_GPIOB_CLK_ENABLE();  
//		GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 |GPIO_PIN_12|GPIO_PIN_13;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
//    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
//		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
//		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
//		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
//		 
//	}
//	//四种模式	
//	void Motor_SetSpeed1(int Speed)
//{
//	 
//    if(Speed > 255) Speed = 255;
//    else if(Speed < -255) Speed = -255;
//    if(Speed>=0)
//    {
//        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET);
//        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);
//        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, Speed);
//    }
//    else{
//        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);
//        HAL_GPIO_WritePin (GPIOA,GPIO_PIN_0,GPIO_PIN_RESET);   
//        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, -Speed);
//    }
//}
//void Motor_SetSpeed2(int Speed)
//{
//	 
//    if(Speed > 255) Speed = 255;
//    else if(Speed < -255) Speed = -255;
//    if(Speed>=0)
//    {
//        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET);
//        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);
//        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, Speed);
//    }
//    else{
//        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);
//        HAL_GPIO_WritePin (GPIOA,GPIO_PIN_0,GPIO_PIN_RESET);   
//        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, -Speed);
//    }
//}

//void Motor_SetSpeed3(int Speed)

//{
//    if(Speed > 255) Speed = 255;
//    else if(Speed < -255) Speed = -255;
//    if(Speed>=0)
//    {
//        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
//        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET);
//        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, Speed);
//    }
//    else{
//        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);
//        HAL_GPIO_WritePin (GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);   
//        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, -Speed);
//    }
//}
//void Motor_SetSpeed4(int Speed)
//{
//	 
//    if(Speed > 255) Speed = 255;
//    else if(Speed < -255) Speed = -255;
//    if(Speed>=0)
//    {
//        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
//        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET);
//        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, Speed);
//    }
//    else{
//        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);
//        HAL_GPIO_WritePin (GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);   
//        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, -Speed);
//    }
//}
//	
//	 void Move_RightFront(int Speed) 
//		 {    
//    Motor_SetSpeed1(Speed);         
//    Motor_SetSpeed2(0);             
//    Motor_SetSpeed3(0);            
//    Motor_SetSpeed4(Speed);          
//     }

//void Move_LeftFront(int Speed) 
//{     
//    Motor_SetSpeed1(0);           
//    Motor_SetSpeed2(Speed);         
//    Motor_SetSpeed3(Speed);          
//    Motor_SetSpeed4(0);            
//}

//void Move_RightBack(int Speed)
//{    
//    Motor_SetSpeed1(0);             
//    Motor_SetSpeed2(-Speed);         
//    Motor_SetSpeed3(-Speed);         
//    Motor_SetSpeed4(0);            
//}

//void Move_LeftBack(int Speed) 
//{      
//    Motor_SetSpeed1(-Speed);         
//    Motor_SetSpeed2(0);              
//    Motor_SetSpeed3(0);              
//    Motor_SetSpeed4(-Speed);         
//}
//void Move_Front(int Speed)
//		{
//    Motor_SetSpeed1(Speed);
//		Motor_SetSpeed2(Speed);
//		Motor_SetSpeed3(Speed);
//		Motor_SetSpeed4(Speed);
//		
//		}	
//  
//void Move_Back(int Speed)
//{
//	  Motor_SetSpeed1(-Speed);
//	  Motor_SetSpeed2(-Speed);
//		Motor_SetSpeed3(-Speed);
//		Motor_SetSpeed4(-Speed);
//		
//}
//void Move_Right(int Speed)
//{
//    Motor_SetSpeed1(Speed);
//	  Motor_SetSpeed2(-Speed);
//		Motor_SetSpeed3(-Speed);
//		Motor_SetSpeed4(Speed);
//	  
//   
//}
//void Move_Left(int Speed)
//{
//    Motor_SetSpeed1(-Speed);
//	  Motor_SetSpeed2(Speed);
//		Motor_SetSpeed3(Speed);
//		Motor_SetSpeed4(-Speed);
//    
//}
//void Move_Right_Onspot(int Speed)
//	{
//    Motor_SetSpeed1(-Speed);  
//    Motor_SetSpeed2(-Speed);
//		Motor_SetSpeed3(Speed);
//		Motor_SetSpeed4(Speed);
//    
//}


//void Move_Left_Onspot(int Speed) 
//	{
//    Motor_SetSpeed1(Speed);   
//		Motor_SetSpeed2(Speed);
//		Motor_SetSpeed3(-Speed);
//		Motor_SetSpeed4(-Speed);
//}



//void Move_Stop(void)
//{
//    Motor_SetSpeed1(0);
//	  Motor_SetSpeed2(0);
//	  Motor_SetSpeed3(0);
//	  Motor_SetSpeed4(0);
//	  
//}



#include "gpio.h"
#include "main.h"
#include "tim.h"
#include "motor.h"
//HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); 
//HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);  
//HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);  
//HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);  

typedef enum {
    MOTOR_1 = 0,
    MOTOR_2,
    MOTOR_3,
    MOTOR_4,
    MOTOR_COUNT
} Motor_TypeDef;

typedef struct {
    GPIO_TypeDef* GPIOx;
    uint16_t IN1_Pin;
    uint16_t IN2_Pin;
    TIM_HandleTypeDef* htim;
    uint32_t channel;
} Motor_Config;

Motor_Config motor_configs[MOTOR_COUNT] = {
    // Motor 1: PA0-IN1, PA1-IN2, TIM1_CH1
    {GPIOA, GPIO_PIN_0, GPIO_PIN_1, &htim3, TIM_CHANNEL_1},
    // Motor 2: PA4-IN1, PA5-IN2, TIM1_CH4
    {GPIOA, GPIO_PIN_4, GPIO_PIN_5, &htim3, TIM_CHANNEL_2},
    // Motor 3: PB0-IN1, PB1-IN2, TIM3_CH1
    {GPIOB, GPIO_PIN_0, GPIO_PIN_1, &htim1, TIM_CHANNEL_1},
    // Motor 4: PB12-IN1, PB13-IN2, TIM3_CH2
    {GPIOB, GPIO_PIN_12, GPIO_PIN_13, &htim1, TIM_CHANNEL_4}
};



void Motor_SetSpeed(Motor_TypeDef motor, int Speed)
{
    if(motor >= MOTOR_COUNT) return;
    
    Motor_Config config = motor_configs[motor];
    
    if(Speed > 255) Speed = 255;
    else if(Speed < -255) Speed = -255;

    if(Speed >= 0) {
        HAL_GPIO_WritePin(config.GPIOx, config.IN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(config.GPIOx, config.IN2_Pin, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(config.GPIOx, config.IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(config.GPIOx, config.IN2_Pin, GPIO_PIN_SET);
        Speed = -Speed;
    }
    __HAL_TIM_SET_COMPARE(config.htim, config.channel, Speed);
}


void Move_Front(int Speed)
{
    Motor_SetSpeed(MOTOR_1, Speed);
    Motor_SetSpeed(MOTOR_2, Speed);
    Motor_SetSpeed(MOTOR_3, Speed);
    Motor_SetSpeed(MOTOR_4, Speed);
}

void Move_Back(int Speed)
{
    Motor_SetSpeed(MOTOR_1, -Speed);
    Motor_SetSpeed(MOTOR_2, -Speed);
    Motor_SetSpeed(MOTOR_3, -Speed);
    Motor_SetSpeed(MOTOR_4, -Speed);
}

void Move_Right(int Speed)
{
    Motor_SetSpeed(MOTOR_1, Speed);
    Motor_SetSpeed(MOTOR_2, -Speed);
    Motor_SetSpeed(MOTOR_3, -Speed);
    Motor_SetSpeed(MOTOR_4, Speed);
}
void Move_Left(int Speed)
{
    Motor_SetSpeed(MOTOR_1, -Speed);
    Motor_SetSpeed(MOTOR_2, Speed);
    Motor_SetSpeed(MOTOR_3, Speed);
    Motor_SetSpeed(MOTOR_4, -Speed);
}

void Move_RightFront(int Speed)
{
    Motor_SetSpeed(MOTOR_1, Speed);
    Motor_SetSpeed(MOTOR_2, 0);
    Motor_SetSpeed(MOTOR_3, 0);
    Motor_SetSpeed(MOTOR_4, Speed);
}

void Move_LeftFront(int Speed)
{
    Motor_SetSpeed(MOTOR_1, 0);
    Motor_SetSpeed(MOTOR_2, Speed);
    Motor_SetSpeed(MOTOR_3, Speed);
    Motor_SetSpeed(MOTOR_4, 0);
}

void Move_Stop(void)
{
    for(int i = 0; i < MOTOR_COUNT; i++) {
        Motor_SetSpeed(i, 0);
    }
}
 
//while (1)
//{
//		Debug_MovementTest();
//		HAL_Delay(2000);
//    
//}
void Debug_MovementTest(void)
		{
		Move_Front (200);
			HAL_Delay(1000);
			Move_Stop ();
			Move_Left (200);
			HAL_Delay(1000);
			Move_Stop ();
			Move_RightFront(180);
			HAL_Delay(1000);
			Move_Stop ();
		}
