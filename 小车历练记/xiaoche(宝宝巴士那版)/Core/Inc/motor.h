#ifdef MOTOR_H
#define MOTOR_H


void Motor_SetSpeed(Motor_TypeDef motor, int Speed);
void Move_Front(int Speed);
void Move_Back(int Speed);
void Move_Right(int Speed);
void Move_Left(int Speed);
void Move_RightFront(int Speed);
void Move_LeftFront(int Speed);
void Move_Stop(void);
void Debug_MovementTest(void);

#endif
