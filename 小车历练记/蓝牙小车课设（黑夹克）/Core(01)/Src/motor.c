#include "motor.h"
#include <string.h>
#include <stdio.h>  // ����stdio.h��ʹ��snprintf
#include "uart.h"  
#include <stdlib.h>  // ����stdlib.h��ʹ��atoi����
// �����ⲿ����
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

//// ������Ͷ���
//typedef enum {
//    MOTOR_1,
//    MOTOR_2,
//    MOTOR_3,
//    MOTOR_4,
//    MOTOR_COUNT
//} Motor_TypeDef;

// ������ýṹ��
typedef struct {
    GPIO_TypeDef* GPIOx;
    uint16_t IN1_Pin;
    uint16_t IN2_Pin;
    TIM_HandleTypeDef* htim;
    uint32_t channel;
} Motor_Config;


Motor_Config motor_configs[MOTOR_COUNT] = {
    // Motor 1: PA0-IN1, PA1-IN2 , TIM2_CH1
    {GPIOA, GPIO_PIN_0, GPIO_PIN_1, &htim2, TIM_CHANNEL_1},
    // Motor 2: PA4-IN1, PA5-IN2, TIM2_CH2
    {GPIOA, GPIO_PIN_4, GPIO_PIN_5, &htim2, TIM_CHANNEL_2},
    // Motor 3: PB0-IN1, PB1-IN2, TIM3_CH1
    {GPIOB, GPIO_PIN_0, GPIO_PIN_1, &htim3, TIM_CHANNEL_1},
    // Motor 4: PB12-IN1, PB13-IN2, TIM3_CH2
    {GPIOB, GPIO_PIN_12, GPIO_PIN_13, &htim3, TIM_CHANNEL_2}
};

// �����ʼ��
void Motor_Init(void)
{
    // ��ʼ�����е����GPIO
    for(int i = 0; i < MOTOR_COUNT; i++) {
        Motor_Config config = motor_configs[i];
        
        // ��ʼ��GPIO����
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = config.IN1_Pin | config.IN2_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(config.GPIOx, &GPIO_InitStruct);
        
        // ��ʼ����Ϊֹͣ״̬
        HAL_GPIO_WritePin(config.GPIOx, config.IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(config.GPIOx, config.IN2_Pin, GPIO_PIN_RESET);
    }
    
    // ����PWM
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
}

//�ٶȺ���
void Motor_SetSpeed(Motor_TypeDef motor, int Speed)
{
    if(motor >= MOTOR_COUNT) return;
    
    Motor_Config config = motor_configs[motor];
    
    if(Speed > 255) Speed = 255;
    else if(Speed < -255) Speed = -255;
    
		if(Speed > 0) {
        HAL_GPIO_WritePin(config.GPIOx, config.IN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(config.GPIOx, config.IN2_Pin, GPIO_PIN_RESET);//��ת
    } else if(Speed < 0){
        HAL_GPIO_WritePin(config.GPIOx, config.IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(config.GPIOx, config.IN2_Pin, GPIO_PIN_SET);//��ת
        Speed = -Speed;
    }else if(Speed==0){
	      HAL_GPIO_WritePin(config.GPIOx, config.IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(config.GPIOx, config.IN2_Pin, GPIO_PIN_RESET);	//ֹͣ,IN1/IN2 ͬʱ SET �� ��Ӳɲ���������˲��ͣ��������Ӳ�������Բ���ܵ��·�ת�л�ʱ��ƽ��ͻ��
//�Ż�����Ϊͬʱ RESET������ֹͣ����ͨ��)
		}
		
    __HAL_TIM_SET_COMPARE(config.htim, config.channel, Speed);
}


void Move_Front(int Speed)
{
    Motor_SetSpeed(MOTOR_1, Speed);
    Motor_SetSpeed(MOTOR_2, -Speed);
    Motor_SetSpeed(MOTOR_3, -Speed);
    Motor_SetSpeed(MOTOR_4, Speed);
}

void Move_Back(int Speed)
{
    Motor_SetSpeed(MOTOR_1, -Speed);
    Motor_SetSpeed(MOTOR_2, Speed);
    Motor_SetSpeed(MOTOR_3, Speed);
    Motor_SetSpeed(MOTOR_4, -Speed);
}

void Move_Right(int Speed)
{
    Motor_SetSpeed(MOTOR_1, Speed);
    Motor_SetSpeed(MOTOR_2, Speed);
    Motor_SetSpeed(MOTOR_3, Speed);
    Motor_SetSpeed(MOTOR_4, Speed);
}
void Move_Left(int Speed)
{
    Motor_SetSpeed(MOTOR_1, -Speed);
    Motor_SetSpeed(MOTOR_2, -Speed);
    Motor_SetSpeed(MOTOR_3, -Speed);
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
        Motor_SetSpeed((Motor_TypeDef)i, 0); // ǿ��ת��ΪMotor_TypeDef
    }
}
 //������Բ���
typedef enum {
    TEST_STOPPED,
    TEST_MOVING_FORWARD,
    TEST_TURNING_LEFT,
    TEST_MOVING_RIGHT_FRONT
} TestState;

TestState currentTestState = TEST_STOPPED;
uint32_t testStartTime = 0;


    // �򵥵��������

// �����˶�����
void Debug_MovementTest(void)
{
    Move_Front(200);
    HAL_Delay(1000);
    Move_Stop();
    Move_Left(200);
    HAL_Delay(1000);
    Move_Stop();
    Move_RightFront(180);
    HAL_Delay(1000);
    Move_Stop();
}

void Debug_MovementTest_NonBlocking(void)
{
    uint32_t currentTime = HAL_GetTick();
    
    switch(currentTestState) {
        case TEST_STOPPED:
            Move_Front(200);
            testStartTime = currentTime;
            currentTestState = TEST_MOVING_FORWARD;
            break;
            
        case TEST_MOVING_FORWARD:
            if(currentTime - testStartTime >= 1000) {
                Move_Stop();
                Move_Left(200);
                testStartTime = currentTime;
                currentTestState = TEST_TURNING_LEFT;
            }
            break;
            
        case TEST_TURNING_LEFT:
            if(currentTime - testStartTime >= 1000) {
                Move_Stop();
                Move_RightFront(180);
                testStartTime = currentTime;
                currentTestState = TEST_MOVING_RIGHT_FRONT;
            }
            break;
            
        case TEST_MOVING_RIGHT_FRONT:
            if(currentTime - testStartTime >= 1000) {
                Move_Stop();
                currentTestState = TEST_STOPPED;
            }
            break;
    }
}
