#ifndef __TYPES_H
#define __TYPES_H

#include <stdint.h>

// 只保留枚举定义
typedef enum {
    VOICE_CMD_NONE = 0,
    VOICE_CMD_FORWARD,
    VOICE_CMD_BACKWARD,
    VOICE_CMD_STOP,
    VOICE_CMD_RESISTANCE,
    VOICE_CMD_SPEED_UP,
    VOICE_CMD_SPEED_DOWN,
    VOICE_CMD_MODE_CHANGE,
    VOICE_CMD_EMERGENCY_STOP,
    VOICE_CMD_TEST
} VoiceCommand_t;

typedef enum {
    DISPLAY_NORMAL = 0,
    DISPLAY_MOTOR_INFO,
    DISPLAY_REHAB_MODE,
    DISPLAY_ERROR
} DisplayMode_t;

#endif /* __TYPES_H */
