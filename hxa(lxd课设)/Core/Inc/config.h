/*
 * config.h
 *
 *  Created on: 2024.5.15
 *      Author: user
 */

#ifndef INC_CONFIG_H_    
#define INC_CONFIG_H_

// =====================================
// 系统配置参数
// =====================================

// 调试功能开关
#define DEBUG_ENABLED           0       // 1: 启用调试输出, 0: 禁用调试输出

// 系统配置
#define SYSTEM_TICK_MS          10      // 系统滴答时间(ms)

// 调试宏定义
#if DEBUG_ENABLED
  #include <stdio.h>
  #define DEBUG_PRINT(fmt, ...)    printf(fmt, ##__VA_ARGS__)
#else
  #define DEBUG_PRINT(fmt, ...)
#endif

// =====================================
// 导轨与位置限制配置
// =====================================

// 物理导轨里程（单位：mm）
#ifndef PHYSICAL_TRAVEL_RANGE_MM
#define PHYSICAL_TRAVEL_RANGE_MM   205.0f   // 实际导轨里程
#endif

#ifndef TRAVEL_RANGE_MM
#define TRAVEL_RANGE_MM            200.0f   // 保险使用范围（200mm）
#endif

#ifndef SAFETY_MARGIN_MM
#define SAFETY_MARGIN_MM            2.5f     // 安全余量（mm）
#endif

// =====================================
// 电机与编码器配置
// =====================================

// 同步带轮参数
#ifndef PULLEY_TEETH
#define PULLEY_TEETH               24      // 同步带轮齿数
#endif

#ifndef BELT_PITCH_MM
#define BELT_PITCH_MM              3.0f    // 同步带节距（mm）
#endif

// 编码器配置
#ifndef ENCODER_RESOLUTION
#define ENCODER_RESOLUTION         11      // 编码器分辨率（每转脉冲数）
#endif

#ifndef GEAR_RATIO
#define GEAR_RATIO                 30.0f   // 齿轮比
#endif

// 计算每毫米对应的编码器计数
// 每转移动距离 = 齿数 * 节距
// 每转编码器计数 = 分辨率 * 齿轮比
// 每毫米计数 = 每转编码器计数 / 每转移动距离
#ifndef ENCODER_COUNTS_PER_MM
#define ENCODER_COUNTS_PER_MM      (float)(ENCODER_RESOLUTION * GEAR_RATIO) / (PULLEY_TEETH * BELT_PITCH_MM)
#endif

// 电机PWM配置
#ifndef MOTOR_PWM_FREQUENCY
#define MOTOR_PWM_FREQUENCY        1000    // PWM频率（Hz）
#endif

#ifndef MOTOR_PWM_MAX_VALUE
#define MOTOR_PWM_MAX_VALUE        1000    // PWM最大计数值
#endif

#ifndef MOTOR_PWM_MIN_DUTY
#define MOTOR_PWM_MIN_DUTY         100     // PWM最小占空比
#endif

#ifndef MOTOR_PWM_MAX_DUTY
#define MOTOR_PWM_MAX_DUTY         900     // PWM最大占空比
#endif

// =====================================
// 康复模式配置
// =====================================

// 阻力等级配置（1-10级）
#define REHAB_RESISTANCE_LEVELS    10      // 阻力等级数量

// 默认阻力等级
#define DEFAULT_RESISTANCE_LEVEL   3       // 默认阻力等级
#define REHAB_DEFAULT_LEVEL        DEFAULT_RESISTANCE_LEVEL

// 阻力系数范围（0-100）
#define MIN_RESISTANCE_COEFFICIENT  10     // 最小阻力系数
#define MAX_RESISTANCE_COEFFICIENT  90     // 最大阻力系数
#define REHAB_MIN_RESISTANCE        MIN_RESISTANCE_COEFFICIENT
#define REHAB_MAX_RESISTANCE        MAX_RESISTANCE_COEFFICIENT

// 位置检测灵敏度（mm/s）
#define MOVEMENT_DETECTION_THRESHOLD 0.5f  // 移动检测阈值

// 阻力响应时间（ms）
#define RESISTANCE_RESPONSE_TIME    50     // 阻力响应时间

// 康复模式目标范围
#define REHAB_TARGET_RANGE_MM       100.0f // 康复模式目标移动范围（mm）

// =====================================
// 蓝牙通信配置
// =====================================

#ifndef BT_CMD_QUEUE_SIZE
#define BT_CMD_QUEUE_SIZE          10      // 蓝牙命令队列大小
#endif
#ifndef BT_BAUDRATE
#define BT_BAUDRATE                9600    // 蓝牙通信波特率
#endif

// =====================================
// 语音模块配置
// =====================================

#ifndef VOICE_BAUDRATE
#define VOICE_BAUDRATE             115200  // 语音模块通信波特率
#endif
#ifndef VOICE_RX_BUFFER_SIZE
#define VOICE_RX_BUFFER_SIZE       32      // 语音接收缓冲区大小
#endif

// =====================================
// OLED显示配置
// =====================================

#define OLED_REFRESH_INTERVAL      100     // OLED刷新间隔（ms）

#endif /* INC_CONFIG_H_ */
