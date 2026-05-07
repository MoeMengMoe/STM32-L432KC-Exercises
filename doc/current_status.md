# 当前状态

## 硬件配置
- **主控**: STM32L432KC (NUCLEO-L432KC), Cortex-M4F @ ~72MHz
- **Flash**: 256KB — 已使用 ~57KB (22%)
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
| EXTI 按键 | ✅ 稳定 | PB5, 带 LED 指示 |
| 温度传感器 EXTI | ✅ 稳定 | TEMP_ARM, 控制红色 LED |
| printf → USART1 | ✅ 稳定 | `_write()` 重定向 |

## 实验性/未验证功能

| 功能 | 状态 | 说明 |
|------|------|------|
| 超声波测距 (HC-SR04) | ⏳ 代码样例 | 输入捕获模式, 未实机验证 |
| 循迹模块 (ETR) | ⏳ 代码样例 | 定时器 ETR 模式, 未实机验证 |
| 呼吸灯 (PWM) | ⏳ 代码样例 | PWM 输出, 未实机验证 |
| 旋转编码器 | ⏳ 代码样例 | EXTI 中断, 未实机验证 |
| PWM 电机驱动 | ⏳ 代码样例 | PWM 输出, 未实机验证 |

## 驱动架构
- **LibDriver 模式**: 所有外设驱动（SSD1306、AHT20、BMP280）均使用 LibDriver
- **LINK 机制**: 通过函数指针将 HAL 实现注入驱动 handle
- **interface 模板**: 位于 `Drivers/Middlewares/libdriver_xxx/interface/`，填入 HAL I2C 调用

## I2C 总线 (hi2c3)
| 设备 | 地址 (8-bit) | 类型 |
|------|-------------|------|
| SSD1306 OLED | 0x78 | 显示 |
| AHT20 | 0x70 | 温湿度 |
| BMP280 | 0xEC | 气压 |

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
