//#ifndef MOTOR_H
//#define MOTOR_H
//void Motor_Init(void);
//void Motor_SetSpeed1(int Speed);
//void Motor_SetSpeed2(int Speed);
//void Motor_SetSpeed3(int Speed);
//void Motor_SetSpeed4(int Speed);
//void Move_Front(int Speed);
//void Move_Back(int Speed);
//void Move_Right(int Speed);
//void Move_Left(int Speed);
//void Move_Stop(void);
//void Move_Right_Onspot(int Speed);
//void Move_Left_Onspot(int Speed);
//void Move_RightFront(int Speed); 
//void Move_LeftFront(int Speed); 
//void Move_RightBack(int Speed); 
//void Move_LeftBack(int Speed);      
//   
//#endif


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

