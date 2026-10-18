#include "control.h"
#include "uart.h"
#include "usart.h"
#include "motor.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define RACE_SPEED       120
#define TURN_SPEED       100

#define LEFT_SPEED_ADJUST   8
#define RIGHT_SPEED_ADJUST  0

#define STOP_DELAY        200

#define A_STRAIGHT1       10000
#define A_TURN            1
#define A_TURN_TIME       1200
#define A_STRAIGHT2       2400

#define B_STRAIGHT1       8000
#define B_TURN            2
#define B_TURN_TIME       225
#define B_STRAIGHT2       3500

#define C_STRAIGHT1       6000
#define C_TURN            2
#define C_TURN_TIME       1000
#define C_STRAIGHT2       6000

typedef enum {
    DIR_STOP,
    DIR_FORWARD,
    DIR_BACKWARD,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_RIGHT_FRONT,
    DIR_LEFT_FRONT
} Move_Direction;

static OperationMode current_mode = MODE_MANUAL;
static RaceState race_state = RACE_IDLE;
static uint32_t race_start_time = 0;
static uint8_t race_step = 0;


// ������ƺ���
void Move_Direction_Control(Move_Direction direction, uint8_t speed)
{
	// ֻ�����ֶ�ģʽ�²���Ӧ�������
    if (GetOperationMode() == MODE_MANUAL) {
    switch(direction) {
        case DIR_STOP:
            Move_Stop();
            break;
        case DIR_FORWARD:
            Move_Front(speed);
            break;
        case DIR_BACKWARD:
            Move_Back(speed);
            break;
        case DIR_LEFT:
            Move_Left(speed);
            break;
        case DIR_RIGHT:
            Move_Right(speed);
            break;
        case DIR_RIGHT_FRONT:
            Move_RightFront(speed);
            break;
        case DIR_LEFT_FRONT:
            Move_LeftFront(speed);
            break;
        default:
            Move_Stop();
            break;
			}
    }
}

// ���ò���ģʽ
void SetOperationMode(OperationMode mode)
{
    if (mode != current_mode) {
        current_mode = mode;
        Move_Stop(); // �л�ģʽʱֹͣС��
        
        if (mode == MODE_AUTO) {
            UART_SendString(&bluetooth_uart, "Switched to AUTO mode (Obstacle Avoidance)\n");
        } else {
            UART_SendString(&bluetooth_uart, "Switched to MANUAL mode (Bluetooth Control)\n");
        }
    }
}

// ��ȡ��ǰ����ģʽ
OperationMode GetOperationMode(void)
{
    return current_mode;
}


void Bluetooth_CommandCallback(uint8_t* command, uint16_t len)
{
    Bluetooth_ProcessCommand(command, len);
}

void Bluetooth_Control_Init(void)
{
    UART_SendString(&bluetooth_uart, "===== RESCUE CAR RACE =====\n");
    UART_SendString(&bluetooth_uart, "=== RACE Commands ===\n");
    UART_SendString(&bluetooth_uart, "- Press '1' or 'A' : Go to Point A\n");
    UART_SendString(&bluetooth_uart, "- Press '2' or 'B' : Go to Point B\n");
    UART_SendString(&bluetooth_uart, "- Press '3' or 'C' : Go to Point C\n");
    UART_SendString(&bluetooth_uart, "=== MANUAL Commands ===\n");
    UART_SendString(&bluetooth_uart, "- 'M' : Manual mode\n");
    UART_SendString(&bluetooth_uart, "- 'Fxx' : Forward\n");
    UART_SendString(&bluetooth_uart, "- 'Bxx' : Backward\n");
    UART_SendString(&bluetooth_uart, "- 'Lxx' : Left\n");
    UART_SendString(&bluetooth_uart, "- 'Rxx' : Right\n");
    UART_SendString(&bluetooth_uart, "- 'S' : Stop\n");
}

void Bluetooth_ProcessCommand(uint8_t* command, uint16_t len)
{
    if(len == 0) return;
    
    Move_Direction direction = DIR_STOP;
    uint8_t speed = 100;
    
    if (command[0] == '1' || command[0] == 'A' || command[0] == 'a') {
        Race_GoToPointA();
        return;
    } else if (command[0] == '2' || command[0] == 'B' || command[0] == 'b') {
        Race_GoToPointB();
        return;
    } else if (command[0] == '3' || command[0] == 'C' || command[0] == 'c') {
        Race_GoToPointC();
        return;
    }
    
    if (command[0] == 'A' || command[0] == 'a') {
        SetOperationMode(MODE_AUTO);
        UART_SendString(&bluetooth_uart,"Switched to AUTO mode\n");
        return;
    } else if (command[0] == 'M' || command[0] == 'm') {
        SetOperationMode(MODE_MANUAL);
        UART_SendString(&bluetooth_uart, "Switched to MANUAL mode\n");
        return;
    }
    
    if (GetOperationMode() != MODE_MANUAL) {
        UART_SendString(&bluetooth_uart, "Switch to MANUAL mode first (send 'M')\n");
        return;
    }
		
	
    // ��������
    switch(command[0]) {
        case 'F': direction = DIR_FORWARD; break;
        case 'B': direction = DIR_BACKWARD; break;
        case 'L': direction = DIR_LEFT; break;
        case 'R': direction = DIR_RIGHT; break;
        case 'Q': direction = DIR_LEFT_FRONT; break; // ��ǰ
        case 'E': direction = DIR_RIGHT_FRONT; break; // ��ǰ
        case 'S': direction = DIR_STOP; break;
        default: 
            // ��Ч������ʹ�����Ϣ
            UART_SendString(&bluetooth_uart, "Invalid command\n");
            return; // ��Ч����
    }
    
    // �����ٶȣ�����У�
    if(len > 1) {
        char speed_str[4] = {0};
        strncpy(speed_str, (char*)command + 1, len - 1);
        speed = atoi(speed_str); // ���ַ���ת��Ϊ����
        if(speed > 255) speed = 255; // ��������ٶ�
    }
    
    // ִ������
    Move_Direction_Control(direction, speed);
    
    // ����ȷ����Ϣ
    char response[50];
    const char* dir_str = "";
    switch(direction) {
        case DIR_FORWARD: dir_str = "Forward"; break;
        case DIR_BACKWARD: dir_str = "Backward"; break;
        case DIR_LEFT: dir_str = "Left"; break;
        case DIR_RIGHT: dir_str = "Right"; break;
        case DIR_LEFT_FRONT: dir_str = "Left Front"; break;
        case DIR_RIGHT_FRONT: dir_str = "Right Front"; break;
        case DIR_STOP: dir_str = "Stop"; break;
        default: break;
    }
    
    snprintf(response, sizeof(response), "Moving %s at speed %d\n", dir_str, speed);
    UART_SendString(&bluetooth_uart, response);
///*��λ�����Է���*/
//		    // ����ģ��ִ����Ϣ
//    snprintf(response, sizeof(response), "[SIMULATION] Moving %s at speed %d\n", dir_str, speed);
//    UART_SendString(&bluetooth_uart, response);
//    
//    // ���Ӷ����ģ����Ϣ
//    if (direction != DIR_STOP) {
//        snprintf(response, sizeof(response), "[SIMULATION] Motor 1: %d, Motor 2: %d, Motor 3: %d, Motor 4: %d\n", 
//                speed, speed, speed, speed);
//        UART_SendString(&bluetooth_uart, response);
//    }
}

static void Race_MoveStraight(void)
{
    int speed_left = RACE_SPEED + LEFT_SPEED_ADJUST;
    int speed_right = RACE_SPEED + RIGHT_SPEED_ADJUST;
    
    Motor_SetSpeed(MOTOR_1, speed_left);
    Motor_SetSpeed(MOTOR_2, -speed_right);
    Motor_SetSpeed(MOTOR_3, -speed_right);
    Motor_SetSpeed(MOTOR_4, speed_left);
}



void Race_GoToPointA(void)
{
    SetOperationMode(MODE_RACE);
    race_state = RACE_GO_TO_A;
    race_start_time = HAL_GetTick();
    race_step = 0;
    UART_SendString(&bluetooth_uart, "RACE: Going to Point A...\n");
}

void Race_GoToPointB(void)
{
    SetOperationMode(MODE_RACE);
    race_state = RACE_GO_TO_B;
    race_start_time = HAL_GetTick();
    race_step = 0;
    UART_SendString(&bluetooth_uart, "RACE: Going to Point B...\n");
}

void Race_GoToPointC(void)
{
    SetOperationMode(MODE_RACE);
    race_state = RACE_GO_TO_C;
    race_start_time = HAL_GetTick();
    race_step = 0;
    UART_SendString(&bluetooth_uart, "RACE: Going to Point C...\n");
}



void Race_Process(void)
{
    if(GetOperationMode() != MODE_RACE) return;
    
    uint32_t elapsed = HAL_GetTick() - race_start_time;
    
    switch(race_state) {
        case RACE_GO_TO_A:
            if(elapsed < A_STRAIGHT1) {
                Race_MoveStraight();
            } else if(A_TURN == 0) {
                Move_Stop();
                race_state = RACE_FINISHED;
                UART_SendString(&bluetooth_uart, "RACE: 到达 A 点！\n");
            } else if(elapsed < A_STRAIGHT1 + STOP_DELAY) {
                Move_Stop();
            } else if(elapsed < A_STRAIGHT1 + STOP_DELAY + A_TURN_TIME) {
                A_TURN == 1 ? Move_Left(TURN_SPEED) : Move_Right(TURN_SPEED);
            } else if(elapsed < A_STRAIGHT1 + STOP_DELAY + A_TURN_TIME + A_STRAIGHT2) {
                Race_MoveStraight();
            } else {
                Move_Stop();
                race_state = RACE_FINISHED;
                UART_SendString(&bluetooth_uart, "RACE: Arrived at Point A!\n");
            }
            break;
            
        case RACE_GO_TO_B:
            if(elapsed < B_STRAIGHT1) {
                Race_MoveStraight();
            } else if(B_TURN == 0) {
                Move_Stop();
                race_state = RACE_FINISHED;
                UART_SendString(&bluetooth_uart, "RACE: Arrived at Point B!\n");
            } else if(elapsed < B_STRAIGHT1 + STOP_DELAY) {
                Move_Stop();
            } else if(elapsed < B_STRAIGHT1 + STOP_DELAY + B_TURN_TIME) {
                B_TURN == 1 ? Move_Left(TURN_SPEED) : Move_Right(TURN_SPEED);
            } else if(elapsed < B_STRAIGHT1 + STOP_DELAY + B_TURN_TIME + B_STRAIGHT2) {
                Race_MoveStraight();
            } else {
                Move_Stop();
                race_state = RACE_FINISHED;
                UART_SendString(&bluetooth_uart, "RACE: Arrived at Point B!\n");
            }
            break;
            
        case RACE_GO_TO_C:
            if(elapsed < C_STRAIGHT1) {
                Race_MoveStraight();
            } else if(C_TURN == 0) {
                Move_Stop();
                race_state = RACE_FINISHED;
                UART_SendString(&bluetooth_uart, "RACE: Arrived at Point C!\n");
            } else if(elapsed < C_STRAIGHT1 + STOP_DELAY) {
                Move_Stop();
            } else if(elapsed < C_STRAIGHT1 + STOP_DELAY + C_TURN_TIME) {
                C_TURN == 1 ? Move_Left(TURN_SPEED) : Move_Right(TURN_SPEED);
            } else if(elapsed < C_STRAIGHT1 + STOP_DELAY + C_TURN_TIME + C_STRAIGHT2) {
                Race_MoveStraight();
            } else {
                Move_Stop();
                race_state = RACE_FINISHED;
                UART_SendString(&bluetooth_uart, "RACE: Arrived at Point C!\n");
            }
            break;
            
        case RACE_FINISHED:
            SetOperationMode(MODE_MANUAL);
            race_state = RACE_IDLE;
            race_step = 0;
            break;
            
        default:
            break;
    }
}
