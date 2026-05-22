# 开发日志

## 2026-04-06 — Initial Commit
项目初始化。STM32CubeMX 生成基础工程，配置 CMake + Ninja 构建系统。

## 2026-04-06 — SSD1306 OLED 驱动成功
- 移植 LibDriver SSD1306 驱动，I2C3 通信成功
- 实现 OLED 显示（0.96' 128x64）
- 通过 interface 模板文件填入 HAL I2C 调用

## 2026-04-08 — 循迹模块与 ETR
- 配置定时器 ETR 模式实现外部脉冲计数（代码样例，未实机验证）

## 2026-04-09 — 输入捕获超声波测距
- 定时器输入捕获实现 HC-SR04 超声波测距（代码样例，未实机验证）

## 2026-04-09 — 呼吸灯实验
- PWM 输出实现 LED 呼吸灯效果（代码样例，未实机验证）

## 2026-04-10 — 旋转编码器实验
- 外部中断实现旋转编码器读取（代码样例，未实机验证）

## 2026-04-12 — PWM 电机旋转
- PWM 驱动直流电机（代码样例，未实机验证）

## 2026-04-28 — RTC 功能 + 温度传感器 EXTI
- 完成 RTC 读写封装（`rtc_app.h/c`）：GET/SET/SET_BY_STRING/SET_BY_STRUCT
- 温度传感器（TEMP_ARM）外部中断，区分 KEY 和温度触发
- 串口 scanf 读取尚未实现

## 2026-05-06 — 简单修补
- 小修补和清理

## 2026-05-07 — 环形缓冲区 + RTC 显示 + 串口协议
- 使用环形缓冲区解决 UART 接收阻塞问题
- 修改 LED 亮灯逻辑（红绿灯顺序）
- RTC 时间显示写入主循环
- **修复**：RTC 影子寄存器导致时间读取不准，需先读 Time 再读 Date
- **修复**：串口输入校验加强
- TODO：SSD1306 改为 GRAM 缓冲区刷新

## 2026-05-07 — 修复 I2C 速率 + 刷新速度
- 提高 I2C 传输速率
- 修复串口发送无响应问题

## 2026-05-07 — 修复时间显示位置
- 修正时间显示错误与刷新位置
- TODO：提升刷新速度，使用缓存刷新

## 2026-05-07 — 集成 AHT20 + BMP280 (本次会话)
- 使用 LibDriver 驱动 AHT20（温湿度）和 BMP280（气压）
- 填入 interface 模板：HAL_I2C_Master_Transmit/Receive（AHT20）和 HAL_I2C_Mem_Read/Write（BMP280）
- 扩展 CMakeLists.txt 添加三个 LibDriver 的 include 和 GLOB_RECURSE
- OLED 显示状态机：TIME_PAGE（日期时间）和 STATUS_PAGE（传感器数据）通过按键切换
- **修复**：主循环 `ssd1306_basic_clear()` 导致闪屏 → 改为直接覆写
- **修复**：ISR 中做 I2C 清屏（ssd1306_basic_clear）不稳定 → 改为标志位在主循环处理
- **修复**：`snprintf` 缓冲区不足导致的编译警告
- **优化**：传感器读取仅在 STATUS_PAGE 时执行，避免I2C总线浪费

## 2026-05-08 — 风扇控制功能开发（进行中）
- 硬件切换：旋转编码器 → PWM 电位器（TIM2_CH1/PA0 输入捕获）
- 新增 ENC_KEY（PA6）编码器按键，EXTI 下降沿中断
- 风扇控制双模式：MANUAL（电位器直控速度）和 AUTO（温度阈值 25°C 自动调速）
- FAN_Auto_Mode() 分段调速函数：25°C起转 20%、每+3°C递增一档至满速
- STATUS_PAGE 显示 MODE 指示（AUTO/MANUAL）和阈值信息
- **发现**：ENC_KEY 上升沿+上拉导致上电误触发 → 改为下降沿触发
- **发现**：FAN_APP_Update 每轮覆盖电机控制，覆盖 AUTO 模式输出 → 待修复
- **发现**：TIM2 仍为编码器模式，未切换为 PWM 输入捕获 → 待修复
- TODO：TIM2 改 PWM 输入捕获、写入 HAL_TIM_IC_CaptureCallback、增加 FAN_PAGE、I2C Timing 覆盖

## 2026-05-22 — 修复旋转编码器引脚映射
- TIM2 编码器模式改为双通道 `TIM_ENCODERMODE_TI12`
- 编码器 A/CLK 固定使用 PA0(TIM2_CH1)，B/DT 固定使用 PA1(TIM2_CH2)
- **硬件约束**：不可使用 PA5 作为编码器输入；实测 PA5 电平不随旋钮变化，TIM2 计数会只在相邻值之间抖动
- 保留串口诊断输出，打印 TIM2 计数与 PA0/PA1/PA5 电平，便于后续接线排查
