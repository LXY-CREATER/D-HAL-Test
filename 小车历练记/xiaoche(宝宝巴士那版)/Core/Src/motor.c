#include "gpio.h"
#include "main.h"
#include "tim.h"
#include "motor.h"
//HAL_TIM_PWM_Start(&hti2, TIM_CHANNEL_1); 
//HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);  
//HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);  
//HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);  

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
    // Motor 1: PA0-IN1, PA1-IN2 , TIM2_CH4
    {GPIOA, GPIO_PIN_0, GPIO_PIN_1, &htim2, TIM_CHANNEL_4},
    // Motor 2: PA4-IN1, PA5-IN2, TIM2_CH3
    {GPIOA, GPIO_PIN_4, GPIO_PIN_5, &htim2, TIM_CHANNEL_3},
    // Motor 3: PB0-IN1, PB1-IN2, TIM3_CH1
    {GPIOB, GPIO_PIN_0, GPIO_PIN_1, &htim3, TIM_CHANNEL_1},
    // Motor 4: PB12-IN1, PB13-IN2, TIM3_CH2
    {GPIOB, GPIO_PIN_12, GPIO_PIN_13, &htim3, TIM_CHANNEL_2}
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
		