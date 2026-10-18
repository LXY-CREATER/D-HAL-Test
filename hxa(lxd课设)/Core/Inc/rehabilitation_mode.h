/* USER CODE BEGIN Header */

	/**
  ******************************************************************************
  * @file           : rehabilitation_mode.h
  * @brief          : Rehabilitation mode control header file
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __REHABILITATION_MODE_H
#define __REHABILITATION_MODE_H

#include "main.h"
#include "motor_controller.h"
#include "encoder_reader.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

// 康复模式阻力等级
#ifndef REHAB_RESISTANCE_LEVELS
#define REHAB_RESISTANCE_LEVELS    10      // 阻力等级数量
#endif
#ifndef REHAB_DEFAULT_LEVEL
#define REHAB_DEFAULT_LEVEL        3       // 默认阻力等级
#endif
#ifndef REHAB_MIN_RESISTANCE
#define REHAB_MIN_RESISTANCE       10      // 最小阻力系数
#endif
#ifndef REHAB_MAX_RESISTANCE
#define REHAB_MAX_RESISTANCE       90      // 最大阻力系数
#endif

// 康复模式目标范围
#ifndef REHAB_TARGET_RANGE_MM
#define REHAB_TARGET_RANGE_MM      100.0f  // 康复模式目标移动范围（mm）
#endif

// 移动检测阈值
#ifndef MOVEMENT_DETECTION_THRESHOLD
#define MOVEMENT_DETECTION_THRESHOLD 0.5f  // 移动检测阈值（mm/s）
#endif

// 阻力响应时间
#ifndef RESISTANCE_RESPONSE_TIME
#define RESISTANCE_RESPONSE_TIME    50      // 阻力响应时间（ms）
#endif

/* USER CODE END Private defines */

/* USER CODE BEGIN Private typedefs */

// 康复模式类型
typedef enum {
    REHAB_MODE_PASSIVE = 0,      // 被动模式
    REHAB_MODE_RESISTANCE,       // 阻力模式
    REHAB_MODE_ASSISTIVE,        // 辅助模式
    REHAB_MODE_COUNT             // 模式数量
} Rehab_Mode;

// 康复模式状态
typedef enum {
    REHAB_MODE_IDLE = 0,
    REHAB_MODE_ACTIVE,
    REHAB_MODE_PAUSED,
    REHAB_MODE_ERROR
} RehabModeState_t;

// 康复参数
typedef struct {
    uint8_t resistance_level;       // 阻力等级（1-10）
    uint8_t max_resistance;         // 最大阻力系数
    uint8_t min_resistance;         // 最小阻力系数
    float target_range_mm;          // 目标移动范围
    float current_range_mm;         // 当前移动范围
    uint8_t is_active;              // 康复模式是否激活
    uint8_t auto_adjust;            // 自动调整阻力
    uint32_t last_update;           // 最后更新时间
    float position_min_mm;          // 最小位置限制
    float position_max_mm;          // 最大位置限制
    float current_position_mm;      // 当前位置
    float center_position_mm;       // 中心位置
    float target_velocity;          // 目标速度
    float current_velocity;         // 当前速度
    RehabModeState_t state;         // 康复模式状态
} RehabilitationParams_t;

// 阻抗控制参数
typedef struct {
    float kp;                       // 位置增益
    float kd;                       // 速度增益
    float ki;                       // 积分增益
    float target_position;          // 目标位置
    float current_error;            // 当前误差
    float last_error;               // 上一次误差
    float integral_error;           // 积分误差
    float last_velocity;            // 上一次速度（用于计算加速度）
    float output_limit;             // 输出限制
} ImpedanceControlParams_t;

/* USER CODE END Private typedefs */

/* USER CODE BEGIN Private variables */

extern RehabilitationParams_t g_rehab_params;
extern ImpedanceControlParams_t g_impedance_params;

/* USER CODE END Private variables */

/* USER CODE BEGIN Private function prototypes */

// 初始化康复模式
void Rehabilitation_Init(void);
void Rehabilitation_Mode_Init(void);

// 启动康复模式
void Rehabilitation_Mode_Start(void);

// 检查康复模式是否激活
uint8_t Rehabilitation_Is_Active(void);

// 启动/停止康复模式
void Rehabilitation_Start(void);
void Rehabilitation_Stop(void);

// 获取康复模式
Rehab_Mode Rehab_Get_Mode(void);

// 获取康复模式等级
uint8_t Rehabilitation_Get_Level(void);

// 获取阻力等级
uint8_t Rehab_Get_Resistance_Level(void);

// 获取重复次数
uint32_t Rehab_Get_Repetitions(void);
uint32_t Rehabilitation_Get_Repetitions(void);

// 获取训练时间
uint32_t Rehab_Get_Training_Time(void);
uint32_t Rehabilitation_Get_Training_Time(void);

// 获取目标最小值
float Rehab_Get_Target_Min(void);
float Rehabilitation_Get_Min_Position(void);

// 获取目标最大值
float Rehab_Get_Target_Max(void);
float Rehabilitation_Get_Max_Position(void);

// 获取训练次数
uint32_t Rehabilitation_Get_Exercise_Count(void);

// 获取总训练距离
float Rehabilitation_Get_Total_Distance(void);

// 增加康复模式等级
void Rehabilitation_Increase_Level(void);

// 减少康复模式等级
void Rehabilitation_Decrease_Level(void);

// 康复模式处理
void Rehabilitation_Process(void);

// 停止康复模式
void Rehabilitation_Mode_Stop(void);

// 设置阻力等级
void Rehabilitation_Set_Resistance(uint8_t level);

// 设置康复范围
void Rehabilitation_Set_Range_MM(float min_mm, float max_mm);

// 更新康复模式
void Rehabilitation_Mode_Update(void);

// 计算阻力PWM值
uint8_t Rehabilitation_Calculate_Resistance(void);

// 阻抗控制更新
void Impedance_Control_Update(void);

// 初始化阻抗控制参数
void Impedance_Control_Init(void);

// 设置阻抗控制目标位置
void Impedance_Set_Target_Position(float target_mm);

// 启用阻抗控制模式
void Impedance_Mode_Enable(uint8_t enable);

// 阻力模式控制函数
void Resistance_Mode_Start(void);
void Resistance_Mode_Stop(void);
void Resistance_Mode_Update(void);

// 设置阻力等级 (1-10)
void Set_Resistance_Level(uint8_t level);

// 获取当前康复模式
Rehab_Mode Rehab_Get_Mode(void);

// 获取阻力等级
uint8_t Rehab_Get_Resistance_Level(void);

// 获取重复次数
uint32_t Rehab_Get_Repetitions(void);

// 获取训练时间
uint32_t Rehab_Get_Training_Time(void);

// 获取目标最大位置
float Rehab_Get_Target_Max(void);

// 获取目标最小位置
float Rehab_Get_Target_Min(void);

/* USER CODE END Private function prototypes */

#endif /* __REHABILITATION_MODE_H */
