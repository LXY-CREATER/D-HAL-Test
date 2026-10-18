#ifndef __DECISION_LOGIC_H
#define __DECISION_LOGIC_H

#include "main.h"
#include "config.h"

void DecisionLogic_Init(void);
void DecisionLogic_Process(void);
DecisionResult_t* DecisionLogic_GetResult(void);

#endif
