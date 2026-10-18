# 康复训练机器人控制系统 - 功能实现要点

本文档总结了康复训练机器人控制系统的功能实现要点，特别是使用了哪些硬件外设和技术。

## 1. 硬件外设使用

### 1.1 定时器（TIM）

#### 1.1.1 PWM生成（TIM3）
- **功能**：生成PWM信号控制电机速度
- **通道**：TIM3_CH1（PB4）和TIM3_CH2（PB5）
- **频率**：根据需要配置
- **占空比**：0-100%可调
- **使用方式**：
  ```c
  // 启动PWM输出
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  
  // 设置PWM占空比
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, duty_cycle);
  ```

#### 1.1.2 编码器模式（TIM2）
- **功能**：读取编码器信号，测量电机位置和速度
- **模式**：编码器模式3（TI1和TI2都计数）
- **计数方向**：自动根据旋转方向增减
- **使用方式**：
  ```c
  // 启动编码器模式
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
  
  // 读取计数值
  int32_t count = __HAL_TIM_GET_COUNTER(&htim2);
  ```

#### 1.1.3 定时中断（TIM4）
- **功能**：生成定时中断，用于周期性任务
- **频率**：100Hz（10ms周期）
- **使用方式**：
  ```c
  // 启动定时器中断
  HAL_TIM_Base_Start_IT(&htim4);
  
  // 中断回调函数
  void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
      if (htim->Instance == TIM4) {
          // 执行周期性任务
          Periodic_Task();
      }
  }
  ```

### 1.2 通用输入输出（GPIO）

#### 1.2.1 方向控制（PB5）
- **功能**：控制电机的旋转方向
- **模式**：推挽输出
- **使用方式**：
  ```c
  // 设置为低电平（正转）
  GPIOB->BRR = GPIO_PIN_5;
  
  // 设置为高电平（反转）
  GPIOB->BSRR = GPIO_PIN_5;
  ```

#### 1.2.2 紧急停止按钮（PA0）
- **功能**：紧急停止电机
- **模式**：输入，上拉
- **中断**：外部中断0
- **使用方式**：
  ```c
  // 配置外部中断
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  
  // 中断回调函数
  void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
      if (GPIO_Pin == GPIO_PIN_0) {
          // 紧急停止
          Emergency_Stop();
      }
  }
  ```

### 1.3 通用异步收发传输器（UART）

#### 1.3.1 蓝牙通信（USART1）
- **功能**：与蓝牙模块通信，接收命令和发送状态
- **波特率**：9600
- **数据位**：8
- **停止位**：1
- **校验位**：无
- **使用方式**：
  ```c
  // 发送数据
  HAL_UART_Transmit(&huart1, (uint8_t*)data, len, HAL_MAX_DELAY);
  
  // 接收数据（中断方式）
  HAL_UART_Receive_IT(&huart1, (uint8_t*)buffer, 1);
  
  // 接收中断回调函数
  void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
      if (huart->Instance == USART1) {
          // 处理接收到的数据
          Process_Bluetooth_Data(buffer[0]);
          
          // 继续接收
          HAL_UART_Receive_IT(&huart1, (uint8_t*)buffer, 1);
      }
  }
  ```

#### 1.3.2 语音模块通信（USART2）
- **功能**：与语音模块通信，识别语音命令
- **波特率**：9600
- **数据位**：8
- **停止位**：1
- **校验位**：无
- **使用方式**：类似USART1

### 1.4 集成电路总线（I2C）

#### 1.4.1 OLED显示屏（I2C1）
- **功能**：与OLED显示屏通信，显示状态信息
- **频率**：100kHz
- **地址**：0x78（或0x7A）
- **使用方式**：
  ```c
  // 发送数据
  HAL_I2C_Master_Transmit(&hi2c1, address, data, len, HAL_MAX_DELAY);
  
  // 接收数据
  HAL_I2C_Master_Receive(&hi2c1, address, data, len, HAL_MAX_DELAY);
  ```

### 1.5 直接存储器访问（DMA）

#### 1.5.1 UART DMA传输
- **功能**：使用DMA进行UART数据传输，减少CPU负担
- **通道**：USART1_TX（DMA1_Channel4）和USART1_RX（DMA1_Channel5）
- **使用方式**：
  ```c
  // 配置DMA传输
  HAL_UART_Transmit_DMA(&huart1, (uint8_t*)data, len);
  HAL_UART_Receive_DMA(&huart1, (uint8_t*)buffer, len);
  
  // DMA传输完成回调函数
  void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
      if (huart->Instance == USART1) {
          // 传输完成处理
      }
  }
  
  void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
      if (huart->Instance == USART1) {
          // 接收完成处理
      }
  }
  ```

#### 1.5.2 TIM DMA传输
- **功能**：使用DMA进行定时器数据传输，用于PWM波形生成
- **通道**：TIM3_CH1（DMA1_Channel6）
- **使用方式**：
  ```c
  // 配置DMA传输
  HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t*)buffer, len);
  ```

## 2. 软件技术使用

### 2.1 中断处理

#### 2.1.1 外部中断
- **功能**：处理外部事件，如紧急停止按钮
- **优先级**：高优先级
- **使用方式**：
  ```c
  // 配置外部中断
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  
  // 中断处理函数
  void EXTI0_IRQHandler(void) {
      HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
  }
  
  // 中断回调函数
  void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
      if (GPIO_Pin == GPIO_PIN_0) {
          // 紧急停止
          Emergency_Stop();
      }
  }
  ```

#### 2.1.2 定时器中断
- **功能**：处理定时事件，如周期性任务
- **优先级**：中等优先级
- **使用方式**：
  ```c
  // 配置定时器中断
  HAL_TIM_Base_Start_IT(&htim4);
  HAL_NVIC_EnableIRQ(TIM4_IRQn);
  HAL_NVIC_SetPriority(TIM4_IRQn, 1, 0);
  
  // 中断处理函数
  void TIM4_IRQHandler(void) {
      HAL_TIM_IRQHandler(&htim4);
  }
  
  // 中断回调函数
  void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
      if (htim->Instance == TIM4) {
          // 执行周期性任务
          Periodic_Task();
      }
  }
  ```

#### 2.1.3 UART中断
- **功能**：处理UART数据接收和发送
- **优先级**：中等优先级
- **使用方式**：
  ```c
  // 配置UART中断
  HAL_UART_Receive_IT(&huart1, (uint8_t*)buffer, 1);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  HAL_NVIC_SetPriority(USART1_IRQn, 2, 0);
  
  // 中断处理函数
  void USART1_IRQHandler(void) {
      HAL_UART_IRQHandler(&huart1);
  }
  
  // 中断回调函数
  void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
      if (huart->Instance == USART1) {
          // 处理接收到的数据
          Process_Bluetooth_Data(buffer[0]);
          
          // 继续接收
          HAL_UART_Receive_IT(&huart1, (uint8_t*)buffer, 1);
      }
  }
  ```

### 2.2 实时操作系统（RTOS）

#### 2.2.1 任务管理
- **功能**：管理多个任务，实现多任务并发
- **任务**：
  - 蓝牙通信任务
  - 语音识别任务
  - 复健模式任务
  - OLED显示任务
  - 数据记录任务
- **使用方式**：
  ```c
  // 创建任务
  xTaskCreate(Bluetooth_Task, "Bluetooth", 128, NULL, 1, NULL);
  xTaskCreate(Voice_Task, "Voice", 128, NULL, 1, NULL);
  xTaskCreate(Rehab_Task, "Rehab", 128, NULL, 1, NULL);
  xTaskCreate(OLED_Task, "OLED", 128, NULL, 1, NULL);
  xTaskCreate(Data_Task, "Data", 128, NULL, 1, NULL);
  
  // 启动调度器
  vTaskStartScheduler();
  ```

#### 2.2.2 队列管理
- **功能**：在任务之间传递数据
- **队列**：
  - 蓝牙命令队列
  - 语音命令队列
  - 数据记录队列
- **使用方式**：
  ```c
  // 创建队列
  bluetooth_queue = xQueueCreate(10, sizeof(char));
  voice_queue = xQueueCreate(10, sizeof(char));
  data_queue = xQueueCreate(10, sizeof(Data_t));
  
  // 发送数据到队列
  xQueueSend(bluetooth_queue, &cmd, portMAX_DELAY);
  
  // 从队列接收数据
  xQueueReceive(bluetooth_queue, &cmd, portMAX_DELAY);
  ```

#### 2.2.3 信号量管理
- **功能**：同步任务和资源访问
- **信号量**：
  - 电机控制信号量
  - 编码器访问信号量
  - OLED显示信号量
- **使用方式**：
  ```c
  // 创建信号量
  motor_semaphore = xSemaphoreCreateMutex();
  encoder_semaphore = xSemaphoreCreateMutex();
  oled_semaphore = xSemaphoreCreateMutex();
  
  // 获取信号量
  xSemaphoreTake(motor_semaphore, portMAX_DELAY);
  
  // 释放信号量
  xSemaphoreGive(motor_semaphore);
  ```

### 2.3 算法实现

#### 2.3.1 阻抗控制算法
- **功能**：根据用户运动产生相应的阻力
- **公式**：
  ```
  力 = kp * 位移误差 + kd * 速度
  ```
- **参数**：
  - kp（刚度系数）：位置偏离目标位置越大，阻力越大
  - kd（阻尼系数）：运动速度越快，阻力越大
- **使用方式**：
  ```c
  void Impedance_Control_Update(void) {
      float current_position = Encoder_Get_Position_MM();
      float current_velocity = Encoder_Get_Velocity_MM_S();
      
      // 计算位移误差（相对于目标位置）
      float error = target_position - current_position;
      
      // 比例项（刚度）
      float proportional = kp * error;
      
      // 微分项（阻尼）
      float derivative = kd * (-current_velocity);
      
      // 总输出
      float output = proportional + derivative;
      
      // 设置PWM和方向
      Set_Motor_Output(output);
  }
  ```

#### 2.3.2 速度滤波算法
- **功能**：对速度信号进行滤波，减少噪声
- **算法**：滑动平均滤波
- **窗口大小**：10
- **使用方式**：
  ```c
  float Velocity_Filter(float new_velocity) {
      static float velocity_buffer[10];
      static int buffer_index = 0;
      
      // 添加新速度到缓冲区
      velocity_buffer[buffer_index] = new_velocity;
      buffer_index = (buffer_index + 1) % 10;
      
      // 计算平均值
      float sum = 0;
      for (int i = 0; i < 10; i++) {
          sum += velocity_buffer[i];
      }
      
      return sum / 10;
  }
  ```

#### 2.3.3 位置校准算法
- **功能**：校准电机位置，设置零点
- **算法**：线性插值
- **使用方式**：
  ```c
  void Position_Calibration(void) {
      // 移动到极限位置
      Motor_Go_Forward(50);
      while (!Is_Limit_Reached()) {
          // 等待到达极限位置
      }
      Motor_Stop();
      
      // 设置零点
      Encoder_Zero_Position();
      
      // 移动到中心位置
      Motor_Move_To_Position(TRAVEL_RANGE_MM / 2);
  }
  ```

## 3. 系统架构

### 3.1 分层架构
```
┌─────────────────────────────────────────┐
│         应用层（Application）           │
│  - 复健模式控制                         │
│  - 蓝牙命令处理                         │
│  - 语音命令处理                         │
│  - OLED显示控制                         │
└─────────────────────────────────────────┘
┌─────────────────────────────────────────┐
│         服务层（Service）               │
│  - 电机控制服务                         │
│  - 编码器读取服务                       │
│  - 数据记录服务                         │
│  - 错误处理服务                         │
└─────────────────────────────────────────┘
┌─────────────────────────────────────────┐
│         驱动层（Driver）                │
│  - 定时器驱动                           │
│  - GPIO驱动                             │
│  - UART驱动                            │
│  - I2C驱动                             │
│  - DMA驱动                             │
└─────────────────────────────────────────┘
┌─────────────────────────────────────────┐
│         硬件层（Hardware）              │
│  - STM32F103C8T6                       │
│  - 电机驱动模块                         │
│  - 编码器                               │
│  - 蓝牙模块                             │
│  - OLED显示屏                           │
│  - 语音模块                             │
└─────────────────────────────────────────┘
```

### 3.2 数据流
```
用户输入（蓝牙/语音）
    ↓
命令解析（应用层）
    ↓
服务调用（服务层）
    ↓
驱动控制（驱动层）
    ↓
硬件执行（硬件层）
    ↓
传感器反馈（硬件层）
    ↓
数据处理（驱动层）
    ↓
服务处理（服务层）
    ↓
状态更新（应用层）
    ↓
用户反馈（OLED/语音）
```

## 4. 性能优化

### 4.1 代码优化
- **减少函数调用**：减少不必要的函数调用，提高执行效率
- **优化算法**：使用更高效的算法，如快速排序、二分查找等
- **减少内存使用**：减少内存分配和拷贝，使用静态变量和全局变量
- **优化循环**：减少循环中的计算，使用循环展开等技术

### 4.2 硬件优化
- **使用DMA**：使用DMA传输数据，减少CPU负担
- **使用中断**：使用中断处理事件，提高实时性
- **优化时钟**：配置合适的系统时钟，平衡性能和功耗
- **低功耗设计**：实现低功耗模式，如睡眠模式、停止模式等

## 5. 安全和可靠性

### 5.1 安全设计
- **位置限制**：限制电机运动范围，确保安全
- **紧急停止**：实现紧急停止功能，优先级最高
- **故障检测**：检测电机和外设故障，如过流、过压等
- **错误处理**：处理错误情况，避免系统崩溃

### 5.2 可靠性设计
- **看门狗**：使用看门狗防止系统死机
- **数据备份**：备份重要数据，防止数据丢失
- **冗余设计**：关键功能采用冗余设计，如双传感器、双控制器等
- **测试验证**：充分测试，确保系统可靠性

## 6. 未来扩展

### 6.1 功能扩展
- **更多控制模式**：添加更多复健模式，如被动模式、辅助模式等
- **数据记录**：记录复健数据，进行分析和统计
- **远程控制**：实现远程控制功能，如手机APP控制、云端控制等
- **机器学习**：使用机器学习优化复健方案，提高复健效果

### 6.2 性能扩展
- **更高精度**：提高位置和速度测量精度，使用更高分辨率的编码器
- **更快响应**：提高系统响应速度，使用更快的处理器和更高效的算法
- **更低功耗**：降低系统功耗，使用低功耗硬件和优化的软件
- **更可靠**：提高系统可靠性，使用更稳定的硬件和更完善的软件

## 总结

康复训练机器人控制系统的功能实现要点包括：

1. **硬件外设使用**：使用定时器、GPIO、UART、I2C和DMA等外设
2. **软件技术使用**：使用中断处理、RTOS和算法实现等技术
3. **系统架构**：采用分层架构和数据流设计
4. **性能优化**：优化代码和硬件，提高系统性能
5. **安全和可靠性**：实现安全设计和可靠性设计，确保系统安全和可靠
6. **未来扩展**：考虑功能扩展和性能扩展，提高系统的可扩展性

通过使用这些硬件外设和软件技术，可以实现一个高性能、高可靠性的康复训练机器人控制系统。

## 作者

- 姓名：[你的名字]
- 邮箱：[你的邮箱]
- GitHub：[你的GitHub链接]

## 许可证

本项目采用MIT许可证，详见LICENSE文件。
