#ifndef MOTOR_H
#define MOTOR_H

// 统一使用typedef定义电机枚举类型，避免重复声明
typedef enum {
    MOTOR_1,
    MOTOR_2,
    MOTOR_3,
    MOTOR_4,
    MOTOR_COUNT
} Motor_TypeDef;

// 函数声明（修改Motor_SetSpeed的参数类型）
void Motor_Init(void);
void Motor_SetSpeed(Motor_TypeDef motor, int Speed);  // 第一个参数改为Motor_TypeDef
void Move_Front(int Speed);
void Move_Back(int Speed);
void Move_Right(int Speed);
void Move_Left(int Speed);
void Move_RightFront(int Speed);
void Move_LeftFront(int Speed);
void Move_Stop(void);
void Debug_MovementTest(void);
void Debug_MovementTest_NonBlocking(void);

#endif  // MOTOR_H
