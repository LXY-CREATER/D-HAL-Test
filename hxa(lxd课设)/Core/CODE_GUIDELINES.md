# 康复训练机器人控制系统 - 代码编写核心要点

本文档总结了康复训练机器人控制系统的代码编写核心要点，帮助开发者理解和维护这个项目。

## 1. 项目结构和模块化设计

### 1.1 目录结构
```
Core/
├── Inc/              # 头文件目录
│   ├── bluetooth_handler.h
│   ├── config.h
│   ├── encoder_reader.h
│   ├── motor_controller.h
│   ├── oled_display.h
│   ├── rehabilitation_mode.h
│   ├── voice_handler.h
│   └── ...
├── Src/              # 源文件目录
│   ├── bluetooth_handler.c
│   ├── encoder_reader.c
│   ├── main.c
│   ├── motor_controller.c
│   ├── oled_display.c
│   ├── rehabilitation_mode.c
│   ├── voice_handler.c
│   └── ...
├── Makefile          # 编译脚本
└── README.md         # 项目说明
```

### 1.2 模块化设计原则
- **单一职责原则**：每个模块只负责一个功能
- **高内聚低耦合**：模块内部功能紧密相关，模块之间依赖关系简单
- **接口抽象**：通过头文件定义模块接口，隐藏实现细节
- **可扩展性**：模块设计考虑未来功能扩展

## 2. 核心模块代码编写要点

### 2.1 电机控制模块（motor_controller.c）

#### 2.1.1 功能需求
- 正转、反转、停止、刹车功能
- PWM调速（0-100%占空比）
- 方向控制（PB5引脚）
- 位置限制和安全保护

#### 2.1.2 代码结构
```c
// 初始化函数
void Motor_Init(TIM_HandleTypeDef *htim, uint32_t channel1, uint32_t channel2);

// 运动控制函数
void Motor_Go_Forward(uint8_t speed_percent);
void Motor_Go_Backward(uint8_t speed_percent);
void Motor_Stop(void);
void Motor_Brake(void);

// 速度和方向控制函数
void Motor_Set_Speed(uint8_t speed_percent);
void Motor_Set_Direction(MotorDirection_t dir);

// 位置控制函数
void Motor_Set_Position_Limits(float min_mm, float max_mm);
float Motor_Get_Current_Position(void);
```

#### 2.1.3 核心实现要点
- **PWM控制**：使用STM32的定时器生成PWM信号
- **方向控制**：使用GPIO引脚控制电机方向
- **速度计算**：将百分比速度转换为PWM占空比
- **位置限制**：根据编码器读数限制电机运动范围
- **安全保护**：紧急停止和故障处理

#### 2.1.4 代码示例
```c
void Motor_Set_Direction(MotorDirection_t dir)
{
    switch(dir) {
        case MOTOR_FORWARD:
            // PB5低电平表示正转（直接操作寄存器）
            GPIOB->BRR = GPIO_PIN_5;
            break;
            
        case MOTOR_BACKWARD:
            // PB5高电平表示反转（直接操作寄存器）
            GPIOB->BSRR = GPIO_PIN_5;
            break;
            
        case MOTOR_STOP:
            // PB5低电平
            GPIOB->BRR = GPIO_PIN_5;
            break;
            
        default:
            Motor_Stop();
            break;
    }
}
```

### 2.2 编码器读取模块（encoder_reader.c）

#### 2.2.1 功能需求
- 读取电机的位置（毫米）
- 读取电机的速度（毫米/秒）
- 位置校准和归零

#### 2.2.2 代码结构
```c
// 初始化函数
void Encoder_Init(TIM_HandleTypeDef *htim);

// 读取函数
float Encoder_Get_Position_MM(void);
float Encoder_Get_Velocity_MM_S(void);

// 校准函数
void Encoder_Zero_Position(void);
void Encoder_Set_Position_MM(float position_mm);
```

#### 2.2.3 核心实现要点
- **定时器模式**：使用STM32的编码器模式读取编码器信号
- **位置计算**：将编码器计数转换为毫米位置
- **速度计算**：通过计算单位时间内的位置变化得到速度
- **滤波处理**：使用滑动平均或低通滤波处理速度信号
- **校准功能**：允许用户设置零点和校准位置

#### 2.2.4 代码示例
```c
float Encoder_Get_Position_MM(void)
{
    // 读取定时器计数值
    int32_t count = __HAL_TIM_GET_COUNTER(g_encoder_config.htim);
    
    // 将计数值转换为毫米
    float position_mm = (float)count * g_encoder_config.mm_per_count;
    
    return position_mm;
}
```

### 2.3 蓝牙通信模块（bluetooth_handler.c）

#### 2.3.1 功能需求
- 接收蓝牙命令
- 发送状态信息
- 命令队列管理

#### 2.3.2 代码结构
```c
// 初始化函数
void Bluetooth_Init(UART_HandleTypeDef *huart);

// 发送函数
void Bluetooth_Send_String(char *str);
void Bluetooth_Send_Formatted(const char *format, ...);

// 接收处理函数
void Bluetooth_Process_Command(char cmd);
void Bluetooth_Check_Incoming(void);
```

#### 2.3.3 核心实现要点
- **UART通信**：使用STM32的UART外设进行蓝牙通信
- **命令解析**：解析接收到的ASCII命令
- **命令队列**：使用环形缓冲区存储待处理的命令
- **响应发送**：发送状态信息和命令响应
- **错误处理**：处理无效命令和通信错误

#### 2.3.4 代码示例
```c
void Bluetooth_Process_Command(char cmd)
{
    switch(cmd) {
        case BT_CMD_STOP:
            Motor_Stop();
            Bluetooth_Send_String("OK: Stop\r\n");
            break;
            
        case BT_CMD_FORWARD:
            Motor_Go_Forward(50);
            Bluetooth_Send_String("OK: Forward 50%\r\n");
            break;
            
        case BT_CMD_BACKWARD:
            Motor_Go_Backward(50);
            Bluetooth_Send_String("OK: Backward 50%\r\n");
            break;
            
        // 其他命令处理...
            
        default:
            Bluetooth_Send_String("ERROR: Unknown command\r\n");
            break;
    }
}
```

### 2.4 OLED显示模块（oled_display.c）

#### 2.4.1 功能需求
- 显示当前位置、速度、阻力级别
- 显示复健模式状态
- 显示电池电量（如果有）

#### 2.4.2 代码结构
```c
// 初始化函数
void OLED_Init(I2C_HandleTypeDef *hi2c);

// 显示函数
void OLED_Display_Position(float position_mm);
void OLED_Display_Velocity(float velocity_mm_s);
void OLED_Display_Resistance_Level(uint8_t level);
void OLED_Display_Rehab_Mode(uint8_t is_active);

// 更新函数
void OLED_Update_Display(void);
```

#### 2.4.3 核心实现要点
- **I2C通信**：使用STM32的I2C外设与OLED显示屏通信
- **显示驱动**：实现OLED显示屏的驱动函数
- **数据格式化**：将数值转换为字符串显示
- **屏幕刷新**：定期更新显示屏内容
- **低功耗**：实现屏幕休眠和唤醒功能

#### 2.4.4 代码示例
```c
void OLED_Display_Position(float position_mm)
{
    char buffer[16];
    sprintf(buffer, "Pos: %.1fmm", position_mm);
    
    // 在指定位置显示文本
    OLED_Display_String(0, 0, buffer);
}
```

### 2.5 语音识别模块（voice_handler.c）

#### 2.5.1 功能需求
- 识别语音命令
- 控制电机运动
- 提供语音反馈

#### 2.5.2 代码结构
```c
// 初始化函数
void Voice_Init(UART_HandleTypeDef *huart);

// 识别函数
void Voice_Process_Command(char *cmd);
void Voice_Check_Incoming(void);

// 反馈函数
void Voice_Send_Feedback(char *feedback);
```

#### 2.5.3 核心实现要点
- **UART通信**：使用STM32的UART外设与语音模块通信
- **命令识别**：解析语音模块发送的命令
- **语音反馈**：发送语音反馈信息
- **错误处理**：处理识别错误和通信错误

#### 2.5.4 代码示例
```c
void Voice_Process_Command(char *cmd)
{
    if (strcmp(cmd, "FORWARD") == 0) {
        Motor_Go_Forward(50);
        Voice_Send_Feedback("Moving forward");
    } else if (strcmp(cmd, "BACKWARD") == 0) {
        Motor_Go_Backward(50);
        Voice_Send_Feedback("Moving backward");
    } else if (strcmp(cmd, "STOP") == 0) {
        Motor_Stop();
        Voice_Send_Feedback("Stopped");
    } else {
        Voice_Send_Feedback("Unknown command");
    }
}
```

### 2.6 复健模式模块（rehabilitation_mode.c）

#### 2.6.1 功能需求
- 阻抗控制算法
- 阻力级别调整（1-10级）
- 自动调整阻力
- 运动范围限制

#### 2.6.2 代码结构
```c
// 初始化函数
void Rehabilitation_Mode_Init(void);

// 控制函数
void Rehabilitation_Mode_Start(void);
void Rehabilitation_Mode_Stop(void);
void Rehabilitation_Mode_Update(void);

// 阻力调整函数
void Set_Resistance_Level(uint8_t level);
void Increase_Resistance(void);
void Decrease_Resistance(void);
```

#### 2.6.3 核心实现要点
- **阻抗控制**：实现阻抗控制算法，根据用户运动产生阻力
- **阻力级别**：根据阻力级别调整阻抗参数
- **运动范围**：限制电机运动范围，确保安全
- **自动调整**：根据用户表现自动调整阻力级别
- **状态管理**：管理复健模式的状态

#### 2.6.4 代码示例
```c
void Impedance_Control_Update(void)
{
    float current_position = Encoder_Get_Position_MM();
    float current_velocity = Encoder_Get_Velocity_MM_S();
    
    // 计算位移误差（相对于目标位置）
    g_impedance_params.current_error = g_impedance_params.target_position - current_position;
    
    // 比例项（刚度）- 位置偏离越大，阻力越大
    float proportional = g_impedance_params.kp * g_impedance_params.current_error;
    
    // 微分项（阻尼）- 速度越快，阻力越大
    float derivative = g_impedance_params.kd * (-current_velocity);
    
    // 总输出
    float output = proportional + derivative;
    
    // 输出限幅
    if (output > g_impedance_params.output_limit) output = g_impedance_params.output_limit;
    if (output < -g_impedance_params.output_limit) output = -g_impedance_params.output_limit;
    
    // 根据输出设置PWM和方向
    if (output > 5.0f) {
        uint16_t duty = (uint16_t)(output);
        GPIOB->BRR = GPIO_PIN_5;
        Motor_Set_PWM(duty, 0);
    } else if (output < -5.0f) {
        uint16_t duty = (uint16_t)(-output);
        GPIOB->BSRR = GPIO_PIN_5;
        Motor_Set_PWM(duty, 0);
    } else {
        Motor_Stop();
    }
}
```

## 3. 主函数和系统集成

### 3.1 main.c结构
```c
int main(void)
{
    // 初始化HAL库
    HAL_Init();
    
    // 配置系统时钟
    SystemClock_Config();
    
    // 初始化外设
    MX_GPIO_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_USART1_UART_Init();
    MX_I2C1_Init();
    
    // 初始化模块
    Motor_Init(&htim3, TIM_CHANNEL_1, TIM_CHANNEL_2);
    Encoder_Init(&htim2);
    Bluetooth_Init(&huart1);
    OLED_Init(&hi2c1);
    Voice_Init(&huart2);
    Rehabilitation_Mode_Init();
    
    // 主循环
    while (1) {
        // 处理蓝牙命令
        Bluetooth_Check_Incoming();
        
        // 处理语音命令
        Voice_Check_Incoming();
        
        // 更新复健模式
        if (g_rehab_params.is_active) {
            Rehabilitation_Mode_Update();
        }
        
        // 更新OLED显示
        OLED_Update_Display();
        
        // 延时
        HAL_Delay(10);
    }
}
```

### 3.2 系统集成要点
- **初始化顺序**：先初始化外设，再初始化模块
- **主循环设计**：非阻塞式设计，确保实时性
- **模块协作**：模块之间通过函数调用和全局变量协作
- **错误处理**：处理外设错误和模块错误
- **低功耗**：实现低功耗模式

## 4. 代码编写规范

### 4.1 命名规范
- **变量名**：使用驼峰命名法，如`currentPosition`
- **函数名**：使用驼峰命名法，如`Motor_Init`
- **常量名**：使用全大写，下划线分隔，如`MAX_POSITION`
- **宏定义**：使用全大写，下划线分隔，如`PI`
- **结构体名**：使用驼峰命名法，如`MotorConfig_t`

### 4.2 注释规范
- **文件头注释**：说明文件功能、作者、版本等信息
- **函数头注释**：说明函数功能、参数、返回值等信息
- **代码注释**：说明复杂代码的逻辑和实现细节
- **注释风格**：使用`/* */`或`//`注释

### 4.3 代码风格
- **缩进**：使用4个空格缩进
- **换行**：每行代码不超过80个字符
- **空行**：在函数之间和逻辑块之间添加空行
- **括号**：使用K&R风格，左括号与函数名在同一行

### 4.4 错误处理
- **返回值检查**：检查函数返回值，处理错误情况
- **错误码定义**：定义错误码，方便错误处理
- **错误日志**：记录错误信息，方便调试

## 5. 调试和测试

### 5.1 调试方法
- **串口调试**：使用串口输出调试信息
- **断点调试**：使用调试器设置断点
- **示波器调试**：使用示波器观察信号波形
- **逻辑分析仪调试**：使用逻辑分析仪观察数字信号

### 5.2 测试方法
- **单元测试**：测试单个函数或模块
- **集成测试**：测试多个模块的协作
- **系统测试**：测试整个系统的功能
- **性能测试**：测试系统的性能指标

## 6. 性能优化

### 6.1 代码优化
- **减少函数调用**：减少不必要的函数调用
- **优化算法**：使用更高效的算法
- **减少内存使用**：减少内存分配和拷贝
- **优化循环**：减少循环中的计算

### 6.2 硬件优化
- **使用DMA**：使用DMA传输数据，减少CPU负担
- **使用中断**：使用中断处理事件，提高实时性
- **优化时钟**：配置合适的系统时钟
- **低功耗设计**：实现低功耗模式

## 7. 安全和可靠性

### 7.1 安全设计
- **位置限制**：限制电机运动范围，确保安全
- **紧急停止**：实现紧急停止功能
- **故障检测**：检测电机和外设故障
- **错误处理**：处理错误情况，避免系统崩溃

### 7.2 可靠性设计
- **看门狗**：使用看门狗防止系统死机
- **数据备份**：备份重要数据，防止数据丢失
- **冗余设计**：关键功能采用冗余设计
- **测试验证**：充分测试，确保系统可靠性

## 8. 版本控制

### 8.1 Git使用
- **提交规范**：使用有意义的提交信息
- **分支管理**：使用分支管理不同版本
- **标签管理**：使用标签标记重要版本
- **协作开发**：使用Git进行协作开发

### 8.2 版本号规范
- **主版本号**：重大功能更新时增加
- **次版本号**：次要功能更新时增加
- **修订号**： bug修复时增加

## 9. 文档编写

### 9.1 代码文档
- **注释**：在代码中添加注释
- **头文件**：在头文件中说明模块接口
- **示例代码**：提供示例代码

### 9.2 项目文档
- **README.md**：说明项目功能、使用方法等
- **API文档**：说明模块接口和使用方法
- **用户手册**：说明系统使用方法
- **开发文档**：说明系统设计和实现细节

## 10. 未来扩展

### 10.1 功能扩展
- **更多控制模式**：添加更多复健模式
- **数据记录**：记录复健数据，进行分析
- **远程控制**：实现远程控制功能
- **机器学习**：使用机器学习优化复健方案

### 10.2 性能扩展
- **更高精度**：提高位置和速度测量精度
- **更快响应**：提高系统响应速度
- **更低功耗**：降低系统功耗
- **更可靠**：提高系统可靠性

## 总结

康复训练机器人控制系统的代码编写核心要点包括：

1. **模块化设计**：将系统分为多个模块，每个模块负责一个功能
2. **硬件抽象**：使用STM32的HAL库抽象硬件细节
3. **实时性**：确保系统的实时性，满足复健需求
4. **安全性**：实现安全保护功能，确保用户安全
5. **可靠性**：提高系统可靠性，减少故障
6. **可维护性**：编写清晰的代码和文档，方便维护
7. **可扩展性**：设计可扩展的系统，方便未来功能扩展

通过遵循这些核心要点，可以编写出高质量、高性能、高可靠性的康复训练机器人控制系统代码。

## 作者

- 姓名：[你的名字]
- 邮箱：[你的邮箱]
- GitHub：[你的GitHub链接]

## 许可证

本项目采用MIT许可证，详见LICENSE文件。
