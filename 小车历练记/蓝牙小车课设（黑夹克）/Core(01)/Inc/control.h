#ifndef __CONTROL_H
#define __CONTROL_H

#include "main.h"

typedef enum {
    MODE_MANUAL,
    MODE_AUTO,
    MODE_RACE
} OperationMode;

typedef enum {
    RACE_IDLE,
    RACE_GO_TO_A,
    RACE_GO_TO_B,
    RACE_GO_TO_C,
    RACE_FINISHED
} RaceState;

void Race_GoToPointA(void);
void Race_GoToPointB(void);
void Race_GoToPointC(void);
void Race_Process(void);


void Bluetooth_CommandCallback(uint8_t* data, uint16_t len);
void Bluetooth_Control_Init(void);
void Bluetooth_ProcessCommand(uint8_t* command, uint16_t len);
// ģʽ�л���������
void SetOperationMode(OperationMode mode);
OperationMode GetOperationMode(void);
#endif
