#ifdef SERO_H
#define SERO_H


//void Sero_Move (void); //这个函数用来控制舵机转动（周期）
void Servo_SetAngle(TIM_HandleTypeDef *htim, uint32_t channel, float angle);
void Servo_Test(void);
#endif
