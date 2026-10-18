#include "motor_type.h"
#include "config.h"
#include "tim.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
// 外部变量声明
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart1;

// 系统状态全局变量定义
SystemStatus_t system_status;

// 内部变量
static uint16_t encoder_last_value = 0;
static uint32_t encoder_overflow_count = 0;
static int32_t encoder_total_count = 0;
static int16_t encoder_velocity = 0;
static uint8_t resistance_active = 0;
static uint32_t exercise_last_position = 0;
static uint8_t exercise_direction = 0;

// 调试宏
#define DEBUG_PRINT(fmt, ...) \
    do { \
        char debug_msg[128]; \
        sprintf(debug_msg, fmt, ##__VA_ARGS__); \
        HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 100); \
    } while(0)

#define SPRING_CONSTANT 0.1f  // 弹簧系数
#define DAMPING_CONSTANT 0.05f // 阻尼系数

		void Motor_PWM_Init(void)
{
    // 1. 开启AFIO时钟
    __HAL_RCC_AFIO_CLK_ENABLE();
    
    // 2. 部分重映射TIM3到PB4和PB5
    // GPIO_Remap: TIM3_REMAP = 0x02 (部分重映射)
    AFIO->MAPR &= ~AFIO_MAPR_TIM3_REMAP;
    AFIO->MAPR |= AFIO_MAPR_TIM3_REMAP_0;  // 部分重映射 (PB4=CH1, PB5=CH2)
    
    // 3. 启动PWM
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    
    DEBUG_PRINT("[MOTOR] PWM initialized (PB4/5 TIM3_CH1/2)\n");
}

/*******************************************************************************
 * 函数名：Moto_Fast_PWMForword
 * 描述  ：电机正转PWM，快衰减（标准正转）
 * 输入  ：速度值 (0-100)
 * 输出  ：void
 *******************************************************************************/
void Moto_Fast_PWMForword(uint8_t speed)
{
    if(speed > MOTOR_MAX_SPEED) speed = MOTOR_MAX_SPEED;
    
    // 确保TIM3已启动
    if(htim3.State != HAL_TIM_STATE_READY)
    {
        HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    }
    
    // 设置PWM占空比：IN1 = PWM, IN2 = 0
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, speed);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
    
    // 更新系统状态
    system_status.motor_state = MOTOR_FORWARD;
    system_status.motor_speed = speed;
}

/*******************************************************************************
 * 函数名：Moto_Fast_PWMReversal
 * 描述  ：电机反转PWM，快衰减（标准反转）
 * 输入  ：速度值 (0-100)
 * 输出  ：void
 *******************************************************************************/
void Moto_Fast_PWMReversal(uint8_t speed)
{
    if(speed > MOTOR_MAX_SPEED) speed = MOTOR_MAX_SPEED;
    
    // 确保TIM3已启动
    if(htim3.State != HAL_TIM_STATE_READY)
    {
        HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    }
    
    // 设置PWM占空比：IN1 = 0, IN2 = PWM
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, speed);
    
    // 更新系统状态
    system_status.motor_state = MOTOR_BACKWARD;
    system_status.motor_speed = speed;
}

/*******************************************************************************
 * 函数名：Moto_Stop
 * 描述  ：电机停止（自由滑行）
 * 输入  ：void
 * 输出  ：void
 *******************************************************************************/
void Moto_Stop(void)
{
    // 停止PWM输出：IN1 = 0, IN2 = 0
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
    
    // 更新系统状态
    system_status.motor_state = MOTOR_STOPPED;
    system_status.motor_speed = 0;
}

/*******************************************************************************
 * 函数名：Moto_Brake
 * 描述  ：电机刹车
 * 输入  ：void
 * 输出  ：void
 *******************************************************************************/
void Moto_Brake(void)
{
    // 设置两个引脚都为高电平实现刹车：IN1 = 100, IN2 = 100
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 100);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 100);
    
    // 更新系统状态
    system_status.motor_state = MOTOR_BRAKING;
    
    // 刹车后延迟一段时间停止
    HAL_Delay(100);
    Moto_Stop();
}

/*******************************************************************************
 * 函数名：Motor_Forward
 * 描述  ：电机正转（增强版，带调试输出）
 * 输入  ：速度值 (0-100)
 * 输出  ：void
 *******************************************************************************/
void Motor_Forward(uint8_t speed)
{
    DEBUG_PRINT("[MOTOR] Forward at %d%%\n", speed);
    Moto_Fast_PWMForword(speed);
}

/*******************************************************************************
 * 函数名：Motor_Backward
 * 描述  ：电机反转（增强版，带调试输出）
 * 输入  ：速度值 (0-100)
 * 输出  ：void
 *******************************************************************************/
void Motor_Backward(uint8_t speed)
{
    DEBUG_PRINT("[MOTOR] Backward at %d%%\n", speed);
    Moto_Fast_PWMReversal(speed);
}

/*******************************************************************************
 * 函数名：Motor_Stop
 * 描述  ：电机停止
 * 输入  ：void
 * 输出  ：void
 *******************************************************************************/
void Motor_Stop(void)
{
    DEBUG_PRINT("[MOTOR] Stop\n");
    Moto_Stop();
}

/*******************************************************************************
 * 函数名：Motor_Set_Speed
 * 描述  ：设置电机速度（保持当前方向）
 * 输入  ：速度值 (0-100)
 * 输出  ：void
 *******************************************************************************/
void Motor_Set_Speed(uint8_t speed)
{
    if(speed > MOTOR_MAX_SPEED) speed = MOTOR_MAX_SPEED;
    
    // 根据当前状态保持方向
    switch(system_status.motor_state)
    {
        case MOTOR_FORWARD:
            Moto_Fast_PWMForword(speed);
            break;
        case MOTOR_BACKWARD:
            Moto_Fast_PWMReversal(speed);
            break;
        default:
            system_status.motor_speed = speed;
            break;
    }
}

/*******************************************************************************
 * 函数名：Motor_Get_Speed
 * 描述  ：获取当前电机速度
 * 输入  ：void
 * 输出  ：当前速度(0-100)
 *******************************************************************************/
uint8_t Motor_Get_Speed(void)
{
    return system_status.motor_speed;
}

/*******************************************************************************
 * 函数名：Motor_Emergency_Stop
 * 描述  ：紧急停止
 * 输入  ：void
 * 输出  ：void
 *******************************************************************************/
void Motor_Emergency_Stop(void)
{
    DEBUG_PRINT("[MOTOR] Emergency Stop!\n");
    Moto_Brake();
    system_status.emergency_stop = 1;
    system_status.error_code = ERROR_CODE_EMERGENCY_STOP;
}

/*******************************************************************************
 * 函数名：Encoder_Get_Position
 * 描述  ：获取编码器位置
 * 输入  ：void
 * 输出  ：编码器总计数
 *******************************************************************************/
int32_t Encoder_Get_Position(void)
{
    Encoder_Update();
    return encoder_total_count;
}

/*******************************************************************************
 * 函数名：Encoder_Get_Velocity
 * 描述  ：获取编码器速度
 * 输入  ：void
 * 输出  ：编码器速度(计数/秒)
 *******************************************************************************/
int16_t Encoder_Get_Velocity(void)
{
    return encoder_velocity;
}

/*******************************************************************************
 * 函数名：Encoder_Reset
 * 描述  ：重置编码器计数
 * 输入  ：void
 * 输出  ：void
 *******************************************************************************/
void Encoder_Reset(void)
{
    encoder_last_value = 0;
    encoder_overflow_count = 0;
    encoder_total_count = 0;
    encoder_velocity = 0;
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    system_status.encoder_position = 0;
    system_status.encoder_speed = 0;
}

/*******************************************************************************
 * 函数名：Encoder_Update
 * 描述  ：更新编码器数据
 * 输入  ：void
 * 输出  ：void
 *******************************************************************************/
void Encoder_Update(void)
{
    static uint32_t last_update_time = 0;
    uint32_t current_time = HAL_GetTick();
    
    // 每10ms更新一次（100Hz）
    if(current_time - last_update_time < 10)
        return;
    
    float dt = (current_time - last_update_time) / 1000.0f;
    last_update_time = current_time;
    
    // 读取编码器计数值
    uint16_t current_value = __HAL_TIM_GET_COUNTER(&htim2);
    
    // 处理溢出（编码器是16位定时器）
    int16_t delta = current_value - encoder_last_value;
    
    if(delta > 32767)        // 正向溢出
    {
        delta -= 65536;
        encoder_overflow_count--;
    }
    else if(delta < -32768)  // 负向溢出
    {
        delta += 65536;
        encoder_overflow_count++;
    }
    
    // 更新总计数
    encoder_total_count += delta;
    encoder_last_value = current_value;
    
    // 计算瞬时速度（计数/秒）
    if(dt > 0)
    {
        int16_t instant_velocity = (int16_t)(delta / dt);
        
        // 低通滤波平滑速度值
        float alpha = 0.3f;
        encoder_velocity = (int16_t)(alpha * instant_velocity + 
                                     (1 - alpha) * encoder_velocity);
    }
    
    // 更新系统状态
    system_status.encoder_position = encoder_total_count;
    system_status.encoder_speed = encoder_velocity;
    system_status.runtime_ms = current_time;
}

/*******************************************************************************
 * 函数名：Resistance_Mode_Init
 * 描述  ：阻力模式初始化
 * 输入  ：void
 * 输出  ：void
 *******************************************************************************/
void Resistance_Mode_Init(void)
{
    resistance_active = 1;
    exercise_last_position = encoder_total_count;
    exercise_direction = 0;
    
    DEBUG_PRINT("[RES] Resistance mode initialized\n");
}

/*******************************************************************************
 * 函数名：Resistance_Mode_Update
 * 描述  ：阻力模式更新
 * 输入  ：void
 * 输出  ：void
 *******************************************************************************/
void Resistance_Mode_Update(void)
{
    if(!resistance_active || system_status.mode != MODE_REHABILITATION)
        return;
    
    static uint32_t last_update = 0;
    static int32_t equilibrium_position = 0; // 平衡位置
    uint32_t current_time = HAL_GetTick();
    
    if(current_time - last_update < 20) // 50Hz更新
        return;
    
    last_update = current_time;
    
    // 获取当前位置和速度
    int32_t current_pos = Encoder_Get_Position();
    int16_t current_vel = Encoder_Get_Velocity();
    
    // 计算弹簧力（胡克定律）：F = -k * x
    int32_t displacement = current_pos - equilibrium_position;
    float spring_force = -SPRING_CONSTANT * displacement * system_status.resistance_level / 10.0f;
    
    // 计算阻尼力：F = -c * v
    float damping_force = -DAMPING_CONSTANT * current_vel;
    
    // 总阻力
    float total_force = spring_force + damping_force;
    
    // 转换为PWM值（0-100）
    uint8_t resistance_pwm = (uint8_t)fabs(total_force);
    if(resistance_pwm > 80) resistance_pwm = 80;
    if(resistance_pwm < 5 && fabs(total_force) > 0.1f) resistance_pwm = 5;
    
    // 施加阻力（方向与位移方向相反）
    if(displacement > 0)
    {
        Moto_Fast_PWMReversal(resistance_pwm);
    }
    else if(displacement < 0)
    {
        Moto_Fast_PWMForword(resistance_pwm);
    }
    else
    {
        Moto_Stop();
    }
    
    DEBUG_PRINT("[RES] Pos:%ld, Vel:%d, Force:%d%%\n", 
                displacement, current_vel, resistance_pwm);
}

/*******************************************************************************
 * 函数名：Set_Resistance_Level
 * 描述  ：设置阻力等级
 * 输入  ：阻力等级 (0-10)
 * 输出  ：void
 *******************************************************************************/
void Set_Resistance_Level(uint8_t level)
{
    if(level > 10) level = 10;
    system_status.resistance_level = level;
    
    DEBUG_PRINT("[RES] Resistance level set to %d\n", level);
}

/*******************************************************************************
 * 函数名：Get_Resistance_Level
 * 描述  ：获取阻力等级
 * 输入  ：void
 * 输出  ：阻力等级(0-10)
 *******************************************************************************/
uint8_t Get_Resistance_Level(void)
{
    return system_status.resistance_level;
}

/*******************************************************************************
 * 函数名：Motor_Test_Routine
 * 描述  ：电机测试例程
 * 输入  ：void
 * 输出  ：void
 *******************************************************************************/
void Motor_Test_Routine(void)
{
    DEBUG_PRINT("\n=== MOTOR TEST ROUTINE ===\n");
    
    // 1. 正转测试
    DEBUG_PRINT("1. Forward test (30%% speed)...\n");
    Motor_Forward(30);
    HAL_Delay(2000);
    DEBUG_PRINT("   Position: %ld, Velocity: %d\n", 
                Encoder_Get_Position(), Encoder_Get_Velocity());
    
    // 2. 反转测试
    DEBUG_PRINT("2. Backward test (30%% speed)...\n");
    Motor_Backward(30);
    HAL_Delay(2000);
    DEBUG_PRINT("   Position: %ld, Velocity: %d\n", 
                Encoder_Get_Position(), Encoder_Get_Velocity());
    
    // 3. 刹车测试
    DEBUG_PRINT("3. Brake test...\n");
    Moto_Brake();
    HAL_Delay(500);
    
    // 4. 速度渐变测试
    DEBUG_PRINT("4. Speed ramp test...\n");
    for(uint8_t i = 0; i <= 100; i += 20)
    {
        Motor_Forward(i);
        HAL_Delay(200);
    }
    
    // 5. 停止
    DEBUG_PRINT("5. Stop motor...\n");
    Moto_Stop();
    
    // 6. 重置编码器
    DEBUG_PRINT("6. Reset encoder...\n");
    Encoder_Reset();
    
    DEBUG_PRINT("=== TEST COMPLETED ===\n\n");
}

/*******************************************************************************
 * 函数名：System_Init
 * 描述  ：系统初始化
 * 输入  ：void
 * 输出  ：void
 *******************************************************************************/
void System_Init(void)
{
    // 初始化系统状态
    memset(&system_status, 0, sizeof(SystemStatus_t));
    system_status.mode = MODE_BLUETOOTH;
    system_status.motor_state = MOTOR_STOPPED;
    system_status.motor_speed = 0;
    system_status.resistance_level = 3;
    system_status.battery_level = 100;
    system_status.temperature = 25;
    system_status.error_code = ERROR_CODE_NONE;
    system_status.exercise_count = 0;
    system_status.exercise_goal = 50;
    system_status.emergency_stop = 0;
    
    // 初始化编码器
    encoder_last_value = __HAL_TIM_GET_COUNTER(&htim2);
    encoder_overflow_count = 0;
    encoder_total_count = 0;
    encoder_velocity = 0;
    
    // 启动编码器定时器
    if(htim2.State == HAL_TIM_STATE_READY)
    {
        HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
        HAL_TIM_Base_Start(&htim2);
    }
    
    // 启动PWM定时器
    if(htim3.State == HAL_TIM_STATE_READY)
    {
        HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    }
    
    // 初始时停止电机
    Motor_Forward(20);
     Motor_PWM_Init();
    DEBUG_PRINT("[SYS] System initialized\n");
}

/*******************************************************************************
 * 函数名：Main_Task
 * 描述  ：主任务函数
 * 输入  ：void
 * 输出  ：void
 *******************************************************************************/
// 在 motor_type.c 的 Main_Task 函数中添加
void Main_Task(void)
{
    static uint32_t last_encoder_update = 0;
    uint32_t current_time = HAL_GetTick();
    
    // 1. 更新编码器数据（每10ms）
    if(current_time - last_encoder_update >= 10)
    {
        Encoder_Update();
        last_encoder_update = current_time;
    }
    
    // 2. 阻力模式更新（如果处于阻力模式）
    if(system_status.mode == MODE_REHABILITATION)
    {
        Resistance_Mode_Update();
    }
    
//    // 3. 蓝牙参数超时检查（新增）
//    Check_Bluetooth_Param_Timeout();
}

// 新增蓝牙参数超时检查函数
void Check_Bluetooth_Param_Timeout(void)
{
//    static uint8_t param_mode = 0;
//    static uint8_t last_cmd = 0;
//    static uint32_t param_timeout = 0;
//    
//    // 这些变量需要与 Process_Bluetooth_Command 共享
//    // 可以使用全局变量或外部声明
//    
//    extern uint8_t bt_param_mode;  // 需要在 motor_type.h 中声明
//    extern uint8_t bt_last_cmd;
//    extern uint32_t bt_param_timeout;
//    
//    if(bt_param_mode && (HAL_GetTick() - bt_param_timeout > 300))
//    {
//        DEBUG_PRINT("[BT] Parameter timeout for %c (auto cancel)\n", bt_last_cmd);
//        bt_param_mode = 0;
//    }
	 static uint32_t last_check = 0;
    uint32_t current_time = HAL_GetTick();
    
    if(current_time - last_check > 1000)  // 每秒检查一次
    {
        last_check = current_time;
	
		}
 }

/*******************************************************************************
 * 函数名：Send_System_Status
 * 描述  ：发送系统状态到串口
 * 输入  ：void
 * 输出  ：void
 *******************************************************************************/
void Send_System_Status(void)
{
    char status_str[128];
    int len = sprintf(status_str, 
        "STAT:M%d,S%d,P%d,V%d,R%d,B%d,T%d,E%d\n",
        system_status.mode,
        system_status.motor_speed,
        system_status.encoder_position,
        system_status.encoder_speed,
        system_status.resistance_level,
        system_status.battery_level,
        system_status.temperature,
        system_status.error_code);
    
    if(len > 0)
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)status_str, len, 100);
    }
}

/*******************************************************************************
 * 函数名：Safety_Check
 * 描述  ：安全检查
 * 输入  ：void
 * 输出  ：void
 *******************************************************************************/
void Safety_Check(void)
{
    static uint32_t last_check_time = 0;
    static uint8_t stall_counter = 0;
    uint32_t current_time = HAL_GetTick();
    
    // 每100ms检查一次
    if(current_time - last_check_time < 100)
        return;
    
    last_check_time = current_time;
    
    // 检查温度
    if(system_status.temperature > 60)
    {
        system_status.error_code = ERROR_CODE_OVERHEAT;
        DEBUG_PRINT("[SAFETY] Overheat! Temperature: %dC\n", system_status.temperature);
        Motor_Emergency_Stop();
    }
    
    // 检查编码器异常（堵转检测）
    if(system_status.motor_speed > 30 && abs(system_status.encoder_speed) < 10)
    {
        stall_counter++;
        if(stall_counter > 10)
        {
            system_status.error_code = ERROR_CODE_ENCODER_FAILURE;
            DEBUG_PRINT("[SAFETY] Motor stall detected!\n");
            Motor_Emergency_Stop();
            stall_counter = 0;
        }
    }
    else
    {
        stall_counter = 0;
    }
    
    // 检查电池（模拟）
    static uint32_t last_battery_update = 0;
    if(current_time - last_battery_update > 60000)
    {
        if(system_status.battery_level > 0)
            system_status.battery_level--;
        last_battery_update = current_time;
    }
    
    if(system_status.battery_level < 10)
    {
        system_status.error_code = ERROR_CODE_LOW_BATTERY;
        DEBUG_PRINT("[SAFETY] Low battery! %d%%\n", system_status.battery_level);
    }
}

/*******************************************************************************
 * 函数名：Process_Bluetooth_Command
 * 描述  ：处理蓝牙命令
 * 输入  ：命令字符
 * 输出  ：void
 *******************************************************************************/
// 在 motor_type.c 中补全 Process_Bluetooth_Command 函数
void Process_Bluetooth_Command(uint8_t cmd)
{
    static uint8_t last_cmd = 0;
    static uint8_t param_mode = 0;
    static uint8_t param_value = 0;
    static uint32_t param_timeout = 0;
    
    uint32_t current_time = HAL_GetTick();
    
    // 检查参数模式是否超时（300ms超时）
    if(param_mode && (current_time - param_timeout > 300))
    {
        DEBUG_PRINT("[BT] Parameter timeout for %c\n", last_cmd);
        param_mode = 0;
        // 使用默认值
        switch(last_cmd)
        {
            case 'F':
                Motor_Forward(50);  // 默认50%速度
                break;
            case 'B':
                Motor_Backward(50); // 默认50%速度
                break;
            case 'R':
                Resistance_Mode_Init();  // 进入康复模式
                break;
        }
    }
    
    if(param_mode)
    {
        // 处理参数
        if(cmd >= '0' && cmd <= '9')
        {
            param_value = cmd - '0';
            DEBUG_PRINT("[BT] Parameter received: %d for %c\n", param_value, last_cmd);
            
            switch(last_cmd)
            {
                case 'F': // 正转速度 0-9 -> 0-90%
                    system_status.mode = MODE_BLUETOOTH;
                    if(param_value == 0)
                        Motor_Stop();
                    else
                        Motor_Forward(param_value * 10);
                    break;
                    
                case 'B': // 反转速度 0-9 -> 0-90%
                    system_status.mode = MODE_BLUETOOTH;
                    if(param_value == 0)
                        Motor_Stop();
                    else
                        Motor_Backward(param_value * 10);
                    break;
                    
                case 'R': // 阻力等级 0-10
                    if(param_value <= 10)
                    {
                        system_status.mode = MODE_REHABILITATION;
                        Set_Resistance_Level(param_value);
                        Resistance_Mode_Init();
                    }
                    break;
                    
                case 'S': // 停止（不需要参数）
                    Motor_Stop();
                    break;
                    
                case 'M': // 模式切换 0-5
                    if(param_value <= 5)
                    {
                        system_status.mode = (SystemMode_t)param_value;
                        DEBUG_PRINT("[BT] Mode set to %d\n", system_status.mode);
                        
                        // 根据模式进行初始化
                        switch(system_status.mode)
                        {
                            case MODE_REHABILITATION:
                                Resistance_Mode_Init();
                                break;
                            case MODE_BLUETOOTH:
                                // 蓝牙模式不需要特殊初始化
                                break;
                            default:
                                Motor_Stop();
                                break;
                        }
                    }
                    break;
            }
        }
        else
        {
            // 参数不是数字，使用默认值
            DEBUG_PRINT("[BT] Invalid parameter for %c: %c\n", last_cmd, cmd);
            switch(last_cmd)
            {
                case 'F':
                    Motor_Forward(50);
                    break;
                case 'B':
                    Motor_Backward(50);
                    break;
                case 'R':
                    Resistance_Mode_Init();
                    break;
                case 'S':
                    Motor_Stop();
                    break;
            }
        }
        
        param_mode = 0; // 退出参数模式
        return;
    }
    
    // 原始命令处理（单字符命令）
    DEBUG_PRINT("[BT] Command: %c (0x%02X)\n", cmd, cmd);
    
    switch(cmd)
    {
        // 需要参数的命令
        case 'F': // 正转，需要速度参数
        case 'B': // 反转，需要速度参数
        case 'R': // 康复模式，需要阻力等级参数
        case 'M': // 模式切换，需要模式参数
            last_cmd = cmd;
            param_mode = 1; // 进入参数等待模式
            param_timeout = current_time; // 记录开始时间
            DEBUG_PRINT("[BT] Waiting parameter for %c (0-9)\n", cmd);
            
            // 发送提示
            char prompt[32];
            switch(cmd)
            {
                case 'F': sprintf(prompt, "Speed (0-9): "); break;
                case 'B': sprintf(prompt, "Speed (0-9): "); break;
                case 'R': sprintf(prompt, "Resist level (0-9): "); break;
                case 'M': sprintf(prompt, "Mode (0-5): "); break;
            }
            HAL_UART_Transmit(&huart1, (uint8_t*)prompt, strlen(prompt), 100);
            break;
            
        // 不需要参数的命令
//        case CMD_FORWARD:
//            system_status.mode = MODE_BLUETOOTH;
//            Motor_Forward(50);
//            break;
//            
//        case CMD_BACKWARD:
//            system_status.mode = MODE_BLUETOOTH;
//            Motor_Backward(50);
//            break;
//            
//        case CMD_STOP:
//            Motor_Stop();
//            break;
            
        case 'S': // 停止（单字符）
            Motor_Stop();
            break;
            
//        case CMD_SET_RESISTANCE:
//            system_status.mode = MODE_REHABILITATION;
//            Resistance_Mode_Init();
//            break;
            
        case '+':
            if(system_status.motor_speed < 90)
            {
                Motor_Set_Speed(system_status.motor_speed + 10);
                // 发送确认
                char ack[20];
                sprintf(ack, "Speed: %d%%\n", system_status.motor_speed);
                HAL_UART_Transmit(&huart1, (uint8_t*)ack, strlen(ack), 100);
            }
            break;
            
        case '-':
            if(system_status.motor_speed > 10)
            {
                Motor_Set_Speed(system_status.motor_speed - 10);
                // 发送确认
                char ack[20];
                sprintf(ack, "Speed: %d%%\n", system_status.motor_speed);
                HAL_UART_Transmit(&huart1, (uint8_t*)ack, strlen(ack), 100);
            }
            break;
            
        case 'U':
            if(system_status.resistance_level < 10)
            {
                Set_Resistance_Level(system_status.resistance_level + 1);
                // 发送确认
                char ack[20];
                sprintf(ack, "Resistance: %d\n", system_status.resistance_level);
                HAL_UART_Transmit(&huart1, (uint8_t*)ack, strlen(ack), 100);
            }
            break;
            
        case 'D':
            if(system_status.resistance_level > 1)
            {
                Set_Resistance_Level(system_status.resistance_level - 1);
                // 发送确认
                char ack[20];
                sprintf(ack, "Resistance: %d\n", system_status.resistance_level);
                HAL_UART_Transmit(&huart1, (uint8_t*)ack, strlen(ack), 100);
            }
            break;
            
        // 数字命令（0-9），直接设置速度
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            system_status.mode = MODE_BLUETOOTH;
            Motor_Set_Speed((cmd - '0') * 10);
            if(cmd == '0') Motor_Stop();
            
            // 发送确认
            {
                char ack[20];
                if(cmd == '0')
                    sprintf(ack, "Motor stopped\n");
                else
                    sprintf(ack, "Speed set to %d%%\n", (cmd - '0') * 10);
                HAL_UART_Transmit(&huart1, (uint8_t*)ack, strlen(ack), 100);
            }
            break;
            
        case 'T':
            Motor_Test_Routine();
            break;
            
        case 'E':
            Motor_Emergency_Stop();
            break;
            
        case 'X':
            system_status.error_code = ERROR_CODE_NONE;
            system_status.emergency_stop = 0;
            DEBUG_PRINT("[BT] Error reset\n");
            
            // 发送确认
            HAL_UART_Transmit(&huart1, (uint8_t*)"Error reset\n", 12, 100);
            break;
            
        case 'G': // 获取状态
            Send_System_Status();
            break;
            
        case 'C': // 校准模式
            system_status.mode = MODE_CALIBRATION;
            Encoder_Reset();
            Motor_Stop();
            DEBUG_PRINT("[BT] Calibration mode\n");
            
            // 发送确认
            HAL_UART_Transmit(&huart1, (uint8_t*)"Calibration mode\n", 17, 100);
            break;
            
        case '?': // 帮助
        case 'H':
            DEBUG_PRINT("[BT] Help command\n");
            Send_Help_Info();
            break;
            
        default:
            DEBUG_PRINT("[BT] Unknown command: %c\n", cmd);
            
            // 发送错误信息
            char err_msg[40];
            sprintf(err_msg, "Unknown command: %c (0x%02X)\n", cmd, cmd);
            HAL_UART_Transmit(&huart1, (uint8_t*)err_msg, strlen(err_msg), 100);
            break;
    }
    
    // 如果不是等待参数的命令，发送确认
    if(!param_mode)
    {
        char ack[32];
        sprintf(ack, "ACK:%c\n", cmd);
        HAL_UART_Transmit(&huart1, (uint8_t*)ack, strlen(ack), 100);
    }
}

// 新增帮助信息函数
void Send_Help_Info(void)
{
    char help_msg[] = 
        "=== Commands Help ===\n"
        "F: Forward (needs speed 0-9)\n"
        "B: Backward (needs speed 0-9)\n"
        "R: Rehab mode (needs level 0-9)\n"
        "S: Stop motor\n"
        "+: Speed up 10%\n"
        "-: Speed down 10%\n"
        "U: Resistance up\n"
        "D: Resistance down\n"
        "0-9: Direct speed set (0-90%)\n"
        "M: Change mode (needs mode 0-5)\n"
        "T: Test routine\n"
        "E: Emergency stop\n"
        "X: Reset error\n"
        "G: Get status\n"
        "C: Calibration mode\n"
        "?: Help\n"
        "===================\n";
    
    HAL_UART_Transmit(&huart1, (uint8_t*)help_msg, strlen(help_msg), 100);
}
