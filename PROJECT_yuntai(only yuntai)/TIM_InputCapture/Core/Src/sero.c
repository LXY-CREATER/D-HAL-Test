#include "sero.h"

// 舵机参数结构体
typedef struct {
    float min_angle;      // 最小角度 (度)
    float max_angle;      // 最大角度 (度)
    float min_pulse_us;   // 最小脉宽 (微秒)
    float max_pulse_us;   // 最大脉宽 (微秒)
} Servo_Params;

// 两个舵机的参数配置
static Servo_Params servo_params[2] = {
    // 水平舵机参数 (通常范围更大)
    {-90.0, 90.0, 500.0, 2500.0},
    // 垂直舵机参数 (通常范围较小)
    {-30.0, 60.0, 1000.0, 2000.0}
};

// 初始化舵机系统
void Servo_Init(void) {
    // 启动TIM4的PWM通道
    HAL_TIM_PWM_Start(&htim4, SERVO_HORIZONTAL);
    HAL_TIM_PWM_Start(&htim4, SERVO_VERTICAL);
    
    // 设置初始位置
    Servo_SetPosition(SERVO_HORIZONTAL, 0);   // 水平居中
    Servo_SetPosition(SERVO_VERTICAL, 0);     // 垂直水平
}

// 设置舵机角度 (自动限幅)
void Servo_SetAngle(Servo_Channel channel, float angle) {
    int index = (channel == SERVO_HORIZONTAL) ? 0 : 1;
    
    // 角度限幅
    if (angle < servo_params[index].min_angle) 
        angle = servo_params[index].min_angle;
    else if (angle > servo_params[index].max_angle) 
        angle = servo_params[index].max_angle;
    
    Servo_SetPosition(channel, angle);
}

// 设置舵机原始位置 (无限幅)
void Servo_SetPosition(Servo_Channel channel, float angle) {
    int index = (channel == SERVO_HORIZONTAL) ? 0 : 1;
    
    // 计算脉宽 (线性映射)
    float pulse_us = servo_params[index].min_pulse_us + 
                   (angle - servo_params[index].min_angle) * 
                   (servo_params[index].max_pulse_us - servo_params[index].min_pulse_us) /
                   (servo_params[index].max_angle - servo_params[index].min_angle);
    
    // 将脉宽转换为定时器计数值 (时钟频率 = 1MHz → 1计数值 = 1μs)
    uint16_t pulse_ticks = (uint16_t)pulse_us;
    
    // 设置比较寄存器值
    __HAL_TIM_SET_COMPARE(&htim4, channel, pulse_ticks);
}
