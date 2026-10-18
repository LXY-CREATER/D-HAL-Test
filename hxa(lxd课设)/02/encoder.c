#include "encoder.h"
#include "main.h"
#include <math.h>

// ============================================
// 私有函数和变量
// ============================================

// 使用访问函数获取全局状态
static EncoderState* get_encoder_ptr(void) {
    return get_encoder_state();
}

// 静态变量
static int32_t last_raw_count = 0;
static float last_position_mm = 0;

// ============================================
// 初始化函数
// ============================================

// 初始化编码器（简单版本）
void Encoder_Init(TIM_HandleTypeDef* timer)
{
    EncoderState* encoder = get_encoder_ptr();
    
    encoder->timer = timer;
    encoder->count = 0;
    encoder->raw_count = 0;
    encoder->position_mm = 0;
    encoder->velocity_mm_s = 0;
    encoder->last_update_time = HAL_GetTick();
    encoder->reverse = 0;
    encoder->initialized = 1;
    encoder->counts_per_mm = COUNTS_PER_MM;
    
    // 启动编码器模式
    HAL_TIM_Encoder_Start(timer, TIM_CHANNEL_ALL);
    
    // 重置计数器
    __HAL_TIM_SET_COUNTER(timer, 0);
    last_raw_count = 0;
    last_position_mm = 0;
    
    DEBUG_PRINT("Encoder initialized. Counts per mm: %.2f\r\n", encoder->counts_per_mm);
}

// 反初始化编码器
void Encoder_Deinit(void)
{
    EncoderState* encoder = get_encoder_ptr();
    
    if (encoder->timer) {
        HAL_TIM_Encoder_Stop(encoder->timer, TIM_CHANNEL_ALL);
    }
    
    encoder->timer = NULL;
    encoder->initialized = 0;
}

// 初始化编码器（扩展版本）
void Encoder_Init_Ex(TIM_HandleTypeDef* timer, uint16_t cpr, float gear_ratio, float pulley_circumference_mm)
{
    EncoderState* encoder = get_encoder_ptr();
    
    encoder->timer = timer;
    encoder->count = 0;
    encoder->raw_count = 0;
    encoder->position_mm = 0;
    encoder->velocity_mm_s = 0;
    encoder->last_update_time = HAL_GetTick();
    encoder->reverse = 0;
    encoder->initialized = 1;
    
    // 计算每毫米计数
    encoder->counts_per_mm = (float)(cpr * gear_ratio) / pulley_circumference_mm;
    
    // 启动编码器模式
    HAL_TIM_Encoder_Start(timer, TIM_CHANNEL_ALL);
    
    // 重置计数器
    __HAL_TIM_SET_COUNTER(timer, 0);
    last_raw_count = 0;
    last_position_mm = 0;
    
    DEBUG_PRINT("Encoder initialized with custom params:\r\n");
    DEBUG_PRINT("  CPR: %d, Gear ratio: %.1f\r\n", cpr, gear_ratio);
    DEBUG_PRINT("  Pulley circumference: %.1fmm\r\n", pulley_circumference_mm);
    DEBUG_PRINT("  Counts per mm: %.2f\r\n", encoder->counts_per_mm);
}

// ============================================
// 更新函数
// ============================================

void Encoder_Update(void)
{
    EncoderState* encoder = get_encoder_ptr();
    
    if (!encoder->initialized || encoder->timer == NULL) return;
    
    uint32_t current_time = HAL_GetTick();
    uint32_t dt = current_time - encoder->last_update_time;
    
    if (dt == 0) return;  // 避免除零
    
    // 获取原始计数
    int32_t current_raw = (int32_t)__HAL_TIM_GET_COUNTER(encoder->timer);
    
    // 处理16位计数器溢出
    int32_t delta = current_raw - last_raw_count;
    
    // 处理正溢出
    if (delta > 32768) {
        delta -= 65536;
    }
    // 处理负溢出
    else if (delta < -32768) {
        delta += 65536;
    }
    
    // 如果方向反转，取反delta
    if (encoder->reverse) {
        delta = -delta;
    }
    
    // 更新计数
    encoder->count += delta;
    encoder->raw_count = current_raw;
    last_raw_count = current_raw;
    
    // 计算位置（毫米）
    encoder->position_mm = (float)encoder->count / encoder->counts_per_mm;
    
    // 限制位置范围
    if (encoder->position_mm < 0) {
        encoder->position_mm = 0;
        encoder->count = 0;
        if (encoder->timer) {
            __HAL_TIM_SET_COUNTER(encoder->timer, 0);
            last_raw_count = 0;
        }
    } else if (encoder->position_mm > PHYSICAL_MAX_MM) {
        encoder->position_mm = PHYSICAL_MAX_MM;
        encoder->count = (int32_t)(PHYSICAL_MAX_MM * encoder->counts_per_mm);
    }
    
    // 计算速度（mm/s）
    float delta_mm = encoder->position_mm - last_position_mm;
    encoder->velocity_mm_s = (delta_mm * 1000.0f) / dt;
    last_position_mm = encoder->position_mm;
    
    // 更新时间戳
    encoder->last_update_time = current_time;
}

// ============================================
// 获取数据函数
// ============================================

int32_t Encoder_Get_Count(void)
{
    EncoderState* encoder = get_encoder_ptr();
    return encoder->count;
}

int32_t Encoder_Get_Raw_Count(void)
{
    EncoderState* encoder = get_encoder_ptr();
    return encoder->raw_count;
}

float Encoder_Get_Position_MM(void)
{
    EncoderState* encoder = get_encoder_ptr();
    return encoder->position_mm;
}

float Encoder_Get_Position_Percent(void)
{
    EncoderState* encoder = get_encoder_ptr();
    float percent = (encoder->position_mm / TRAVEL_RANGE_MM) * 100.0f;
    return CLAMP(percent, 0, 100);
}

float Encoder_Get_Velocity_MM_S(void)
{
    EncoderState* encoder = get_encoder_ptr();
    return encoder->velocity_mm_s;
}

int32_t Encoder_Get_Velocity(void)
{
    EncoderState* encoder = get_encoder_ptr();
    return (int32_t)(encoder->velocity_mm_s * encoder->counts_per_mm);
}

int32_t Encoder_Get_Delta(void)
{
    static int32_t last_count = 0;
    EncoderState* encoder = get_encoder_ptr();
    int32_t delta = encoder->count - last_count;
    last_count = encoder->count;
    return delta;
}

// ============================================
// 设置/重置函数
// ============================================

void Encoder_Set_Reverse(uint8_t reverse)
{
    EncoderState* encoder = get_encoder_ptr();
    
    if (encoder->reverse != reverse) {
        encoder->reverse = reverse;
        
        // 反转时调整当前计数
        if (reverse) {
            encoder->count = -encoder->count;
            if (encoder->timer) {
                int32_t current = (int32_t)__HAL_TIM_GET_COUNTER(encoder->timer);
                __HAL_TIM_SET_COUNTER(encoder->timer, -current);
                last_raw_count = -current;
            }
        }
        
        DEBUG_PRINT("Encoder direction: %s\r\n", reverse ? "REVERSED" : "NORMAL");
    }
}

void Encoder_Reset_Position_MM(float position_mm)
{
    EncoderState* encoder = get_encoder_ptr();
    
    position_mm = CLAMP(position_mm, 0, TRAVEL_RANGE_MM);
    
    encoder->count = (int32_t)(position_mm * encoder->counts_per_mm);
    encoder->position_mm = position_mm;
    
    if (encoder->timer) {
        int32_t hw_count = encoder->reverse ? -encoder->count : encoder->count;
        __HAL_TIM_SET_COUNTER(encoder->timer, hw_count & 0xFFFF);
        last_raw_count = hw_count & 0xFFFF;
    }
    
    last_position_mm = position_mm;
    
    DEBUG_PRINT("Encoder reset to %.1fmm, count=%ld\r\n", position_mm, encoder->count);
}

void Encoder_Reset(void)
{
    Encoder_Reset_Position_MM(0);
}

void Encoder_Reset_To_Zero(void)
{
    Encoder_Reset();
}

// ============================================
// 转换函数
// ============================================

float Encoder_Counts_To_MM(int32_t counts)
{
    EncoderState* encoder = get_encoder_ptr();
    return (float)counts / encoder->counts_per_mm;
}

int32_t Encoder_MM_To_Counts(float mm)
{
    EncoderState* encoder = get_encoder_ptr();
    return (int32_t)(mm * encoder->counts_per_mm);
}

int32_t Encoder_Percent_To_Counts(float percent)
{
    percent = CLAMP(percent, 0, 100);
    float mm = (percent / 100.0f) * TRAVEL_RANGE_MM;
    return Encoder_MM_To_Counts(mm);
}

// ============================================
// 状态检查函数
// ============================================

uint8_t Encoder_Is_Moving(void)
{
    EncoderState* encoder = get_encoder_ptr();
    return (fabs(encoder->velocity_mm_s) > 0.5f);
}

uint8_t Encoder_Is_At_Limit(void)
{
    EncoderState* encoder = get_encoder_ptr();
    if (encoder->position_mm <= 1.0f) return 1;        // 下限
    if (encoder->position_mm >= TRAVEL_RANGE_MM - 1.0f) return 2;  // 上限
    return 0;                                         // 不在限位
}

uint8_t Encoder_Is_Initialized(void)
{
    EncoderState* encoder = get_encoder_ptr();
    return encoder->initialized;
}

// ============================================
// 校准函数
// ============================================

void Encoder_Direction_Test(void)
{
    DEBUG_PRINT("=== Encoder Direction Test ===\r\n");
    
    EncoderState* encoder = get_encoder_ptr();
    
    // 记录初始状态
    int32_t initial_count = Encoder_Get_Count();
    float initial_mm = Encoder_Get_Position_MM();
    
    DEBUG_PRINT("Initial: count=%ld, pos=%.1fmm\r\n", initial_count, initial_mm);
    
    // 如果位置为负，说明方向反了
    if (initial_mm < -0.5f) {
        DEBUG_PRINT("Negative position detected! Auto-reversing encoder...\r\n");
        Encoder_Set_Reverse(1);
        
        // 重新读取
        initial_mm = Encoder_Get_Position_MM();
        DEBUG_PRINT("After reverse: pos=%.1fmm\r\n", initial_mm);
    }
    
    DEBUG_PRINT("Manual direction test:\r\n");
    DEBUG_PRINT("1. Push the slider forward slightly\r\n");
    DEBUG_PRINT("2. Position should increase\r\n");
    DEBUG_PRINT("3. Pull the slider backward slightly\r\n");
    DEBUG_PRINT("4. Position should decrease\r\n");
    
    DEBUG_PRINT("Encoder direction test complete\r\n");
}

void Encoder_Calibrate(void)
{
    DEBUG_PRINT("Starting encoder calibration...\r\n");
    
    // 重置编码器
    Encoder_Reset();
    
    // 校准方向
    Encoder_Direction_Test();
    
    DEBUG_PRINT("Encoder calibration complete\r\n");
}
