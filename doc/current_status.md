# 当前状态

## 硬件配置
- **主控**: STM32L432KC (NUCLEO-L432KC), Cortex-M4F @ ~72MHz
- **Flash**: 256KB — 已使用 ~57KB (22%) — 注：未启用 newlib-nano 浮点格式化，使用整数转换替代 `%f`
- **RAM**: 48KB + 16KB(RAM2) — 已使用 ~3.5KB (7%)
- **工具链**: arm-none-eabi-gcc 13.3.1 (STM32CubeIDE 捆绑)

## 已实现功能

| 功能 | 状态 | 说明 |
|------|------|------|
| SSD1306 OLED 显示 | ✅ 稳定 | 128x64, I2C3, LibDriver, 支持 FONT_12/FONT_16 |
| USART1 串口调试 | ✅ 稳定 | 115200, printf 重定向到 `_write()` |
| RTC 实时时钟 | ✅ 稳定 | 外部 LSE, 支持串口设置时间 |
| UART 命令协议 | ✅ 稳定 | 格式 `YYYY-MM-DD-HH:MM:SS` 设置 RTC |
| AHT20 温湿度 | ✅ 稳定 | I2C3, LibDriver 驱动 |
| BMP280 气压 | ✅ 稳定 | I2C3, LibDriver 驱动 |
| OLED 页面切换 | ✅ 稳定 | 按键 PB5 切换时间页/传感器页 |
| EXTI 按键 | ✅ 稳定 | PB5 + PA6(ENC_KEY), 带 LED 指示 |
| printf → USART1 | ✅ 稳定 | `_write()` 重定向 |
| DRV8833 电机驱动 | ✅ 稳定 | TIM1_CH3/CH4 PWM, 支持正反转/刹车/滑行 |
| 风扇手动模式 | 🔧 开发中 | 电位器 TIM2_CH1 PWM 输入捕获, 直控速度 |
| 风扇自动模式 | 🔧 开发中 | AHT20 温度 → 分段调速, 阈值 25°C 写死 |

## 实验性/未验证功能

| 功能 | 状态 | 说明 |
|------|------|------|
| 超声波测距 (HC-SR04) | ⏳ 代码样例 | 输入捕获模式, 未实机验证 |
| 循迹模块 (ETR) | ⏳ 代码样例 | 定时器 ETR 模式, 未实机验证 |
| 呼吸灯 (PWM) | ⏳ 代码样例 | PWM 输出, 未实机验证 |

## 风扇控制功能（开发中）

### 硬件
- **PWM 电位器**：TIM2_CH1 (PA0) 输入捕获测量占空比，0-100% 映射速度
- **ENC_KEY**：PA6，EXTI 下降沿中断，STATUS_PAGE 下切换 MANUAL↔AUTO

### 模式
- **MANUAL**：电位器占空比 → 风扇速度，方向只进不退
- **AUTO**：AHT20 温度 → 分段调速（25°C:20%, 28°C:40%, 31°C:60%, 34°C:80%, 37°C:100%）

### 待完成
- TIM2 从编码器模式改为 PWM 输入捕获模式
- 新增 FAN_PAGE（第3页）
- STATUS_PAGE 显示过大需精简
- I2C Timing 覆盖为 100kHz

## 浮点处理策略
- newlib-nano 默认不链接 `printf`/`snprintf` 的 `%f` 支持
- 传感器温度/气压值在读取后 `×10` 转为整数，用 `%d.%d` 格式化输出
- 避免 `-u _printf_float` 链接选项，节省 ~10KB Flash
- 示例：`25.3°C` → `temp_x10 = 253` → `"%d.%d"` → `"25.3"`

## 驱动架构
- **LibDriver 模式**: 所有外设驱动（SSD1306、AHT20、BMP280）均使用 LibDriver
- **LINK 机制**: 通过函数指针将 HAL 实现注入驱动 handle
- **interface 模板**: 位于 `Drivers/Middlewares/libdriver_xxx/interface/`，填入 HAL I2C 调用

## I2C 总线 (hi2c3)
- **I2C 时钟**: ~100kHz (Standard Mode)，在 `USER CODE` 中覆盖 Timing=0x36905E5F
- **原因**: 原始 CubeMX 配置 ~1.7MHz，AHT20/BMP280 combo 模块信号质量有限，降速后稳定

| 设备 | 地址 (8-bit) | 地址 (7-bit) | 类型 |
|------|-------------|-------------|------|
| SSD1306 OLED | 0x78 | 0x3C | 显示 |
| AHT20 | 0x70 | 0x38 | 温湿度 |
| BMP280 | 0xEE | 0x77 | 气压 (ADO_HIGH) |

## OLED 页面布局

### TIME_PAGE (默认)
```
行0: 2026-05-07        (FONT_16)
行1: 12:34:56           (FONT_16)
```

### STATUS_PAGE (按键切换)
```
行0: T:25.5C H:60%      (FONT_16)
行1: P:1013.2hPa        (FONT_16)
```

## 构建命令
```bash
cube-cmake --preset Debug           # 配置
cmake --build build/Debug           # 编译
openocd -f board/st_nucleo_l432kc.cfg -c "program build/Debug/Test_L432KC.elf verify reset exit"  # 烧录
```
