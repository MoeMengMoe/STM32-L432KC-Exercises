# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

STM32L432KC (NUCLEO-L432KC) multifunction embedded firmware project. Cortex-M4F @ ~72MHz, 256KB Flash, 48KB+16KB RAM. Uses STM32CubeMX for pin/peripheral configuration and CMake + Ninja for building.

- **Project**: `stm32-l432kc-embedded-lab` (GitHub: `MoeMengMoe/stm32-l432kc-embedded-lab`)
- **Directory (legacy)**: `Test_L432KC` — rename to `stm32-l432kc-embedded-lab` after re-cloning
- **Legacy remote**: `STM32-L432KC-Exercises` → migrated to `stm32-l432kc-embedded-lab`

## Build & Flash

```bash
# Configure (uses cube-cmake wrapper from STM32CubeIDE)
cube-cmake --preset Debug

# Build
cmake --build build/Debug

# Flash with OpenOCD
openocd -f board/st_nucleo_l432kc.cfg -c "program build/Debug/Test_L432KC.elf verify reset exit"
```

Output artifacts: `.elf`, `.hex`, `.bin` in `build/Debug/`.

## Key Architecture

- **CubeMX-generated code**: Files have `USER CODE BEGIN/END` markers — custom code goes between these to survive regeneration
- **HAL drivers**: `Drivers/STM32L4xx_HAL_Driver/` (I2C, UART, GPIO, DMA, EXTI, etc.)
- **SSD1306 OLED**: `Drivers/Middlewares/libdriver_ssd1306/` — LibDriver-based, uses function-pointer "link" pattern for HAL abstraction. Interface implementation is in `interface/driver_ssd1306_interface_template.c` (I2C via `hi2c3`)
- **printf redirection**: `_write()` in `main.c` → USART1 (`huart1`)
- **UART command protocol**: 2-byte frames (cmd + value). Currently: `L0`/`L1` controls green LED (PB6)

## Hardware Map

| Peripheral | Pins           | Function              |
|------------|----------------|-----------------------|
| USART1     | PA9(TX), PA10(RX) | Debug serial (115200) |
| I2C3       | PA7(SCL), PB4(SDA, NJTRST) | SSD1306 OLED (0x3C)  |
| LED_GREEN  | PB6            | Active high            |
| KEY        | PB5            | Button input           |

## Branch Convention

| Branch                          | Purpose                           |
|---------------------------------|-----------------------------------|
| `main`                          | Stable releases, PR merges only   |
| `develop`                       | Integration branch                |
| `feature/<name>`                | Feature development               |

## Commit Style

Bilingual conventional commits: `type: short description` (English), followed by detailed EN/CN descriptions and `Original: <original message>`.

Types: `feat`, `fix`, `refactor`, `perf`, `chore`, `docs`, `test`.

## Build System

- Toolchain: `arm-none-eabi-gcc` (from STM32CubeIDE bundle, `gnu-tools-for-stm32/13.3.1+st.9`)
- Linker script: `STM32L432XX_FLASH.ld` (FLASH 256K @ 0x08000000, RAM 48K @ 0x20000000, RAM2 16K @ 0x10000000)
- MCU flags: `-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard`
- `cmake/stm32cubemx/CMakeLists.txt` lists all HAL source files — add new HAL modules there
- `CMakeLists.txt` adds libdriver_ssd1306 via GLOB_RECURSE — add user sources in `target_sources`

## lsp配置

- `cube starm-clangd` (from STM32CubeIDE) for code intelligence
- Config in `.vscode/settings.json` and `.clangd` — compilation database at `build/Debug/compile_commands.json`
