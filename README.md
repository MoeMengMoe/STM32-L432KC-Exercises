# STM32L432KC Embedded Lab

多功能嵌入式固件项目，基于 **NUCLEO-L432KC** 开发板 (Cortex-M4F @ 72MHz)。

## Features

| 功能 | 外设 | 状态 |
|------|------|------|
| OLED 显示屏 (SSD1306, 128x64) | I2C3 | Done |
| 温湿度传感器 (AHT20) | I2C3 | Done |
| 气压传感器 (BMP280) | I2C3 | Done |
| 实时时钟 (RTC) | LSE | Done |
| 直流电机驱动 (DRV8833) | TIM1 PWM | Done |
| 风扇控制 (自动/手动) | DRV8833 | Done |
| 蜂鸣器演奏 | TIM15 PWM | Done |
| 串口调试 (115200) | USART1 | Done |
| 按键页面切换 | PB5 EXTI | Done |
| 呼吸灯 | PWM (feature/pwm-breathing-light) | WIP |
| 旋转编码器 | TIM2 Encoder (PA0/PA1) | WIP |
| 超声波测距 (HC-SR04) | TIM Input Capture (feature/timer-input-capture) | WIP |
| 循迹传感器 | TIM2 ETR (feature/timer-etr) | WIP |

## Hardware

| Peripheral | Pins | Function |
|------------|------|----------|
| USART1 | PA9(TX), PA10(RX) | Debug serial (115200) |
| I2C3 | PA7(SCL), PB4(SDA) | SSD1306 + AHT20 + BMP280 (0x3C / 0x38 / 0xEC) |
| LED_GREEN | PB6 | Active high |
| KEY | PB5 | Button input (falling edge) |
| DRV8833 | TIM1 CH1/CH2 | Dual motor PWM |
| Buzzer | TIM15 CH1 | Passive buzzer |

## Quick Start

### Prerequisites

- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) (bundles `arm-none-eabi-gcc` 13.3.1+st.9 and `cube-cmake`)
- OpenOCD (for flashing)
- Ninja build system

### Build & Flash

```bash
# Configure
cube-cmake --preset Debug

# Build
cmake --build build/Debug

# Flash
openocd -f board/st_nucleo_l432kc.cfg -c "program build/Debug/Test_L432KC.elf verify reset exit"
```

Output artifacts: `.elf`, `.hex`, `.bin` in `build/Debug/`.

## Usage

### Serial Commands

Connect at **115200 8N1**. Type a date string to set RTC:

```
2026-05-07-14:30:00
```

### OLED Pages

| Page | Content |
|------|---------|
| TIME_PAGE | Date & time (`YYYY-MM-DD` + `HH:MM:SS`) |
| STATUS_PAGE | Temp/Humidity, Pressure, Fan status |

Press **KEY (PB5)** to toggle pages.

### Fan Control

- **Auto mode**: Fan turns on when temperature exceeds threshold (default 25C)
- **Manual mode**: Control speed via encoder switch (ENC_KEY)
- Toggle modes by pressing encoder switch on STATUS_PAGE
- Rotary encoder A/B must use **PA0 (TIM2_CH1)** and **PA1 (TIM2_CH2)**.
- **Do not use PA5 for the encoder**: PA5 was verified to stay static on this board setup, causing TIM2 to only bounce between adjacent counts.

## Project Structure

```
├── Core/                   # Application code
│   ├── Inc/               # Headers
│   └── Src/               # Sources (main.c, drivers, apps)
├── Drivers/
│   ├── STM32L4xx_HAL_Driver/  # ST HAL
│   └── Middlewares/           # libdriver_ssd1306, libdriver_aht20, libdriver_bmp280
├── CMakeLists.txt             # Top-level build
├── cmake/stm32cubemx/         # CubeMX CMake config
└── Test_L432KC.ioc          # CubeMX project file
```

## Branch Structure

| Branch | Purpose |
|--------|---------|
| `main` | Stable releases |
| `develop` | Integration |
| `feature/*` | Feature development |

## License

STM32 HAL code under ST license. Application code under MIT.
