#include "at8870_driver.h"
#include "main.h"

static TIM_HandleTypeDef* pwm_tim = NULL;
static uint32_t pwm_channel_1 = 0;
static uint32_t pwm_channel_2 = 0;
static uint16_t pwm_period = 1000;
static AT8870_State current_state = AT8870_STOP;

/**
 * @brief  AT8870��ƿ��ʼ��
 * @param  tim: PWMʱ������
 * @param  ch1: IN1����PWMͨ��
 * @param  ch2: IN2����PWMͨ��
 */
void AT8870_Init(TIM_HandleTypeDef* tim, uint32_t ch1, uint32_t ch2)
{
    pwm_tim = tim;
    pwm_channel_1 = ch1;
    pwm_channel_2 = ch2;
    
    // ��ȡPWM����
    pwm_period = __HAL_TIM_GET_AUTORELOAD(tim);
    
    // ��ʼ״̬��ֹͣ������ȫ����Ϊ�͵�ƽ
    // �ر�GPIO���ģʽ����Ϊ�ͨ����ģʽ
    HAL_GPIO_WritePin(MOTOR_IN1_PORT, MOTOR_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_IN2_PORT, MOTOR_IN2_PIN, GPIO_PIN_RESET);
    
    // ֹͣPWM���
    HAL_TIM_PWM_Stop(pwm_tim, pwm_channel_1);
    HAL_TIM_PWM_Stop(pwm_tim, pwm_channel_2);
    
    // �ر�PWM���źŷ���
    __HAL_TIM_SET_COMPARE(pwm_tim, pwm_channel_1, 0);
    __HAL_TIM_SET_COMPARE(pwm_tim, pwm_channel_2, 0);
    
    current_state = AT8870_STOP;
}

/**
 * @brief  ������ֹͣ��ȫ����Ϊ�͵�ƽ��ֹͣPWM
 */
static void AT8870_Full_Stop(void)
{
    // 1. �ȹر�PWM���źŷ���
    __HAL_TIM_SET_COMPARE(pwm_tim, pwm_channel_1, 0);
    __HAL_TIM_SET_COMPARE(pwm_tim, pwm_channel_2, 0);
    
    // 2. ֹͣPWM���
    HAL_TIM_PWM_Stop(pwm_tim, pwm_channel_1);
    HAL_TIM_PWM_Stop(pwm_tim, pwm_channel_2);
    
    // 3. ����GPIOΪ�͵�ƽ
    HAL_GPIO_WritePin(MOTOR_IN1_PORT, MOTOR_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_IN2_PORT, MOTOR_IN2_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  ����AT8870״̬
 * @param  state: ��Ҫ�����״̬
 * @param  pwm_duty: PWMռ�ձ� (0-pwm_period)
 */
void AT8870_Set_State(AT8870_State state, uint16_t pwm_duty)
{
    // �ж�PWMռ�ձ�
    if (pwm_duty > pwm_period) {
        pwm_duty = pwm_period;
    }
    
    // �ȹر�ǰ��״̬
    AT8870_Full_Stop();
    
    current_state = state;
    
    switch (state) {
        case AT8870_STOP:
            // ֹͣģʽ��IN1=0, IN2=0 (Hi-Z) - �Ѿ���AT8870_Full_Stop������
            break;
            
        case AT8870_FORWARD:
            // ǰ��ģʽ��IN1=PWM, IN2=0
            HAL_GPIO_WritePin(MOTOR_IN2_PORT, MOTOR_IN2_PIN, GPIO_PIN_RESET);
            __HAL_TIM_SET_COMPARE(pwm_tim, pwm_channel_1, pwm_duty);
            __HAL_TIM_SET_COMPARE(pwm_tim, pwm_channel_2, 0);
            HAL_TIM_PWM_Start(pwm_tim, pwm_channel_1);
            break;
            
        case AT8870_BACKWARD:
            // ����ģʽ��IN1=0, IN2=PWM
            HAL_GPIO_WritePin(MOTOR_IN1_PORT, MOTOR_IN1_PIN, GPIO_PIN_RESET);
            __HAL_TIM_SET_COMPARE(pwm_tim, pwm_channel_1, 0);
            __HAL_TIM_SET_COMPARE(pwm_tim, pwm_channel_2, pwm_duty);
            HAL_TIM_PWM_Start(pwm_tim, pwm_channel_2);
            break;
            
        case AT8870_BRAKE:
            // ɲ��ģʽ��IN1=1, IN2=1 (GPIO���ģʽ)
            HAL_GPIO_WritePin(MOTOR_IN1_PORT, MOTOR_IN1_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(MOTOR_IN2_PORT, MOTOR_IN2_PIN, GPIO_PIN_SET);
            break;
    }
}

/**
 * @brief  ���ֹͣ��ƿ
 */
void AT8870_Stop(void)
{
    AT8870_Set_State(AT8870_STOP, 0);
}

/**
 * @brief  ���ɲ��ֹͣ
 */
void AT8870_Brake(void)
{
    AT8870_Set_State(AT8870_BRAKE, 0);
}

/**
 * @brief  ���ƽ��ֹͣ(����ֹͣ)
 */
void AT8870_Coast(void)
{
    AT8870_Set_State(AT8870_STOP, 0);
}

/**
 * @brief  ������ֹͣ
 */
void AT8870_Emergency_Stop(void)
{
    AT8870_Brake();
}

/**
 * @brief  ����ָ��PWMռ�ձ�
 * @param  duty_in1: IN1ռ�ձ�
 * @param  duty_in2: IN2ռ�ձ�
 */
void AT8870_Set_PWM(uint16_t duty_in1, uint16_t duty_in2)
{
    if (duty_in1 > pwm_period) duty_in1 = pwm_period;
    if (duty_in2 > pwm_period) duty_in2 = pwm_period;
    
    __HAL_TIM_SET_COMPARE(pwm_tim, pwm_channel_1, duty_in1);
    __HAL_TIM_SET_COMPARE(pwm_tim, pwm_channel_2, duty_in2);
}

/**
 * @brief  ֹͣPWM���
 */
void AT8870_Stop_PWM(void)
{
    HAL_TIM_PWM_Stop(pwm_tim, pwm_channel_1);
    HAL_TIM_PWM_Stop(pwm_tim, pwm_channel_2);
}

/**
 * @brief  ��ʼPWM���
 */
void AT8870_Start_PWM(void)
{
    HAL_TIM_PWM_Start(pwm_tim, pwm_channel_1);
    HAL_TIM_PWM_Start(pwm_tim, pwm_channel_2);
}

/**
 * @brief  ��ȡ��ǰ״̬
 * @retval AT8870״̬
 */
AT8870_State AT8870_Get_State(void)
{
    return current_state;
}

/**
 * @brief  �ж����Ƿ������
 * @retval 1=������, 0=ֹͣ��
 */
uint8_t AT8870_Is_Running(void)
{
    return (current_state == AT8870_FORWARD || current_state == AT8870_BACKWARD);
}
