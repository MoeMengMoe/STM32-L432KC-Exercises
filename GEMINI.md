# STM32L432KC Firmware Project (Test_L432KC)

This project is a firmware implementation for the **NUCLEO-L432KC** development board, featuring an STM32L432KC MCU (Cortex-M4F). It provides a real-time clock (RTC) display on an SSD1306 OLED, UART-based time configuration, and GPIO control.

## Project Overview

- **MCU:** STM32L432KC (~72MHz, 256KB Flash, 64KB RAM).
- **Architecture:** STM32CubeMX generated code with custom application logic in `Core/Src` and `Core/Inc`.
- **Key Features:**
    - SSD1306 OLED display for RTC date/time.
    - UART-based time synchronization.
    - GPIO-based LED control and button/interrupt handling.
    - Serial debugging via `printf` redirection to UART.

## Hardware Mapping

| Peripheral | Pins | Function |
| :--- | :--- | :--- |
| **USART1** | PA9 (TX), PA10 (RX) | Debug Serial (115200) & Command Input |
| **I2C3** | PB4 (SCL), PB3 (SDA) | SSD1306 OLED (Address: 0x3C) |
| **LED_GREEN** | PB6 | Active High LED |
| **LED_RED** | PA6 | Active High LED |
| **KEY** | PB5 | User Button (EXTI interrupt) |
| **TEMP_ARM** | PA8 | Temperature Alarm Input (EXTI interrupt) |
| **RTC** | Internal | Real-Time Clock with LSE/LSI support |

## Build and Flash

The project uses a CMake-based build system, typically invoked via the `cube-cmake` wrapper provided by STM32CubeIDE or similar tools.

### Key Commands

- **Configure Build:**
  ```powershell
  cube-cmake --preset Debug
  ```
- **Build Project:**
  ```powershell
  cmake --build build/Debug
  ```
- **Flash Firmware:**
  ```powershell
  openocd -f board/st_nucleo_l432kc.cfg -c "program build/Debug/Test_L432KC.elf verify reset exit"
  ```

Output artifacts (`.elf`, `.hex`, `.bin`) are generated in the `build/Debug/` directory.

## Development Conventions

- **CubeMX Code Integrity:** Always place custom code within the `/* USER CODE BEGIN ... */` and `/* USER CODE END ... */` markers to ensure it persists after regenerating code from `Test_L432KC.ioc`.
- **OLED Driver:** Uses `libdriver_ssd1306`. Interface implementation is linked via `hi2c3`.
- **RTC Management:** Use the `rtc_app.c/h` interface for getting and setting time.
    - **Time Update Pattern:** Sending a string like `YYYY-MM-DD HH:MM:SS` over UART1 will update the RTC.
- **Error Handling:** Standard `Error_Handler()` is used for critical peripheral failures.
- **Style:** Follows standard C11 conventions. Indentation and naming should match existing CubeMX patterns.

## Key Files

- `Core/Src/main.c`: Application entry point and peripheral initialization.
- `Core/Src/rtc_app.c`: RTC helper functions (Get/Set time).
- `Drivers/Middlewares/libdriver_ssd1306/`: SSD1306 OLED driver library.
- `CMakeLists.txt`: Project build configuration.
- `CLAUDE.md`: Additional technical notes and build instructions.

## Related Documentation

- `.doc/ReadMe.md`: Contains preliminary notes and diagrams for PWM motor control (DRV8833), which may be a planned or related subsystem.
