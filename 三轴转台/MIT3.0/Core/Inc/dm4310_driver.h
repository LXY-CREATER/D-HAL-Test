#ifndef __DM4310_DRIVER_H__
#define __DM4310_DRIVER_H__

#include "config.h"

// º¯ÊýÉùÃ÷
void DM4310_Init(void);
void DM4310_SetTarget(uint8_t motor_id, uint8_t mode, float position, float velocity, float current);
void DM4310_ProcessFeedback(uint32_t std_id, uint8_t *data);
void DM4310_SendSyncCommands(void);

#endif /* __DM4310_DRIVER_H__ */
