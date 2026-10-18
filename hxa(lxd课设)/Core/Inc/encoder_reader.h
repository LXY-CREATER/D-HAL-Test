/* USER CODE BEGIN Header */

	/**
  ******************************************************************************
  * @file           : encoder_reader.h
  * @brief          : Encoder reader header file
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __ENCODER_READER_H
#define __ENCODER_READER_H

#include "main.h"
#include "tim.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

// 编码器参数
#ifndef ENCODER_CPR
#define ENCODER_CPR             600         // 编码器线数 (Counts Per Revolution)
#endif
#ifndef GEAR_RATIO
#define GEAR_RATIO              30.0f       // 减速比
#endif
#ifndef PULLEY_TEETH
#define PULLEY_TEETH            24          // 同步带轮齿数
#endif
#ifndef BELT_PITCH_MM
#define BELT_PITCH_MM           3.0f        // 皮带节距(mm)
#endif
#ifndef PULLEY_CIRCUMFERENCE_MM
#define PULLEY_CIRCUMFERENCE_MM (PULLEY_TEETH * BELT_PITCH_MM)  // 皮带轮周长
#endif
#ifndef TRAVEL_RANGE_MM
#define TRAVEL_RANGE_MM         200.0f      // 有效行程(mm)
#endif

// 计算每毫米的计数
#define COUNTS_PER_MM   ((ENCODER_CPR * GEAR_RATIO * 4.0f) / PULLEY_CIRCUMFERENCE_MM)

// 编码器方向反转
#ifndef ENCODER_REVERSE
#define ENCODER_REVERSE         0           // 1=反转, 0=正常
#endif

/* USER CODE END Private defines */

/* USER CODE BEGIN Private typedefs */

// 编码器数据结构
typedef struct {
    TIM_HandleTypeDef* timer;       // 定时器句柄
    int32_t count;                  // 当前计数
    int32_t prev_count;             // 上一次计数
    float position_mm;              // 当前位置(mm)
    float velocity_mm_s;            // 当前速度(mm/s)
    uint32_t last_update_time;      // 最后更新时间
    uint8_t initialized;            // 是否初始化
} EncoderData_t;

// 全局编码器数据
typedef struct {
    int32_t total_counts;           // 总计数
    int32_t last_count;             // 上一次计数
    int32_t delta_count;            // 计数变化
    uint32_t last_time;             // 最后时间
    int32_t velocity;               // 速度(计数/秒)
    float velocity_mm_per_s;        // 速度(mm/s)
    float position_mm;              // 位置(mm)
    float position_percent;         // 位置百分比(0-100)
} GlobalEncoderData_t;

/* USER CODE END Private typedefs */

/* USER CODE BEGIN Private variables */

extern EncoderData_t s_encoder;
extern GlobalEncoderData_t g_encoder;
extern uint8_t s_encoder_reverse;

/* USER CODE END Private variables */

/* USER CODE BEGIN Private function prototypes */

// 初始化编码器
void Encoder_Init(TIM_HandleTypeDef *htim);
void Encoder_Init_Ex(TIM_HandleTypeDef *htim, uint16_t cpr, float gear_ratio, 
                     float pulley_diameter_mm, float travel_range_mm);

// 更新编码器数据
void Encoder_Update(void);

// 检查编码器是否初始化
uint8_t Encoder_Is_Initialized(void);

// 编码器方向测试
void Encoder_Direction_Test(void);

// 获取编码器数据
int32_t Encoder_Get_Count(void);
float Encoder_Get_Position_MM(void);
float Encoder_Get_Velocity_MM_S(void);
float Encoder_Get_Position_Percent(void);
int32_t Encoder_Get_Position(void);

// 设置零位
void Encoder_Set_Zero(void);
void Encoder_Set_Zero_Position(void);

// 设置方向
void Encoder_Set_Reverse(uint8_t reverse);

/* USER CODE END Private function prototypes */

#endif /* __ENCODER_READER_H */
