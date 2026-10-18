// includes_fixed.h
#ifndef __INCLUDES_FIXED_H
#define __INCLUDES_FIXED_H

// 标准库
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// STM32 HAL
#include "main.h"
#include "gpio.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"

// 项目配置（必须先包含）
#include "config_fixed.h"

// 项目模块（使用修复版本）
#include "bluetooth_handler_fixed.h"
#include "voice_handler_fixed.h"
#include "motor_controller.h"
#include "encoder_reader.h"
#include "oled_display.h"
#include "rehabilitation_mode.h"
#include "system_manager.h"

// 全局变量声明
extern SystemStatus_t g_system;
extern MotorStatus_t g_motor_status;
extern EncoderData_t g_encoder;
extern RehabilitationParams_t g_rehab_params;
extern BluetoothStatus_t g_bluetooth;
extern VoiceStatus_t g_voice;
extern OLEDStatus_t g_oled;

#endif /* __INCLUDES_FIXED_H */
