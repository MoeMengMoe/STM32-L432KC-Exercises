# 项目知识库

## 构建系统

### CMake + Ninja
项目使用 CMake + Ninja 构建，通过 STM32CubeIDE 自带的 `cube-cmake` 脚本配置：

```bash
cube-cmake --preset Debug    # 生成 compile_commands.json + build files
cmake --build build/Debug    # 增量编译
```

- 工具链: `arm-none-eabi-gcc` (GNU Tools for STM32 13.3.1)
- MCU Flags: `-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard`
- C 标准: C11

### CMakeLists.txt 结构

```
add_executable()           → 包含用户源文件（仅头文件声明）
add_subdirectory(cmake/stm32cubemx)  → CubeMX 生成的 HAL 源码
target_include_directories → LibDriver 头文件路径（src + interface + example）
file(GLOB_RECURSE)        → 扫描 LibDriver 的 .c 文件
target_sources()           → 添加扫描到的源码
target_link_libraries()    → 链接 stm32cubemx
POST_BUILD                 → 自动生成 .hex 和 .bin + 打印内存占用
```

**注意**: GLOB_RECURSE 会扫描指定目录下所有 .c 文件（包括 test 目录）。确保 test 文件不包含 `main()` 函数以避免链接冲突。

### 添加新的 LibDriver 模块
以 AHT20/BMP280 为例的添加步骤：
1. 将驱动文件夹放入 `Drivers/Middlewares/`
2. 在 CMakeLists.txt 中添加 `target_include_directories`（三个目录：src、interface、example）
3. 添加 `file(GLOB_RECURSE)` 和 `target_sources()`
4. 补全 `interface/xxx_interface_template.c` 中的 HAL 实现

---

## LibDriver 架构模式

### LINK 机制（函数指针注入）
LibDriver 使用"LINK"宏将底层接口函数注册到驱动 handle 中：

```c
// 定义 handle 时
static aht20_handle_t gs_handle;

// init 函数中绑定函数指针
DRIVER_AHT20_LINK_INIT(&gs_handle, aht20_handle_t);
DRIVER_AHT20_LINK_IIC_INIT(&gs_handle, aht20_interface_iic_init);
DRIVER_AHT20_LINK_IIC_READ_CMD(&gs_handle, aht20_interface_iic_read_cmd);
DRIVER_AHT20_LINK_IIC_WRITE_CMD(&gs_handle, aht20_interface_iic_write_cmd);
DRIVER_AHT20_LINK_DELAY_MS(&gs_handle, aht20_interface_delay_ms);
DRIVER_AHT20_LINK_DEBUG_PRINT(&gs_handle, aht20_interface_debug_print);
```

本质上 LINK 宏就是结构体成员赋值：
```c
#define DRIVER_AHT20_LINK_IIC_INIT(HANDLE, FUC)  (HANDLE)->iic_init = FUC
```

驱动核心层（`src/driver_xxx.c`）只调用 `handle->func()`，不关心底层实现。

### 调用链
```
main.c
  → driver_xxx_basic.c    (示例层: LINK 绑定 + 封装调用)
    → driver_xxx.c        (驱动核心: 调用 handle->函数指针)
      → interface_template.c (用户实现: HAL 外设操作)
        → STM32 HAL 库   (硬件寄存器操作)
```

### Interface 模板实现

**AHT20** (命令式 I2C 协议):
```c
uint8_t aht20_interface_iic_read_cmd(uint8_t addr, uint8_t *buf, uint16_t len) {
    return (HAL_I2C_Master_Receive(&hi2c3, addr, buf, len, 1000) == HAL_OK) ? 0 : 1;
}
uint8_t aht20_interface_iic_write_cmd(uint8_t addr, uint8_t *buf, uint16_t len) {
    return (HAL_I2C_Master_Transmit(&hi2c3, addr, buf, len, 1000) == HAL_OK) ? 0 : 1;
}
```

**BMP280** (寄存器式 I2C 协议):
```c
uint8_t bmp280_interface_iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
    return (HAL_I2C_Mem_Read(&hi2c3, addr, reg, I2C_MEMADD_SIZE_8BIT, buf, len, 1000) == HAL_OK) ? 0 : 1;
}
uint8_t bmp280_interface_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
    return (HAL_I2C_Mem_Write(&hi2c3, addr, reg, I2C_MEMADD_SIZE_8BIT, buf, len, 1000) == HAL_OK) ? 0 : 1;
}
```

**通用函数**:
```c
void xxx_interface_delay_ms(uint32_t ms)  { HAL_Delay(ms); }
void xxx_interface_debug_print(...)        { vprintf(fmt, args); }  // 依赖 printf 重定向
```

---

## CubeMX 生成代码规范

- CubeMX 生成的代码用 `USER CODE BEGIN/END` 包围用户可编辑区域
- **不要修改** USER CODE 标记之外的代码，否则 CubeMX 重新生成时会覆盖
- 用户代码应始终写在 `USER CODE BEGIN xxx` / `USER CODE END xxx` 之间
- 添加新外设时在 CubeMX 中配置并重新生成，保留所有 USER CODE 区域

---

## 硬件知识

### NUCLEO-L432KC 引脚映射 (部分)

| 板载标记 | 引脚 | 功能 |
|---------|------|------|
| D0 | PB4 | I2C3_SDA (NJTRST) |
| D13 | PA7 | I2C3_SCL |
| D1 | PB3 | - |
| D10 | PB6 | LED_GREEN |
| - | PB5 | KEY |
| D6/TX | PA9 | USART1_TX |
| D7/RX | PA10 | USART1_RX |

### I2C 总线共享
多个 I2C 设备可共享同一总线，前提是地址不冲突。当前项目三个设备共用 `hi2c3`：
- SSD1306 (0x3C / 0x78)
- AHT20 (0x38 / 0x70)
- BMP280 (0x76 / 0xEC 或 0x77 / 0xEE)

所有 SCL 并连、SDA 并连。I2C 协议通过地址区分设备。

---

## 代码规范

### 命名
- HAL 外设句柄: `huart1`, `hi2c3`, `hrtc`
- 用户封装函数: `RTC_DATA_GET()`, `SHOW_TIME_PAGE()`
- LibDriver 函数: `aht20_basic_init()`, `bmp280_basic_read()`

### 中断处理原则
1. ISR 中**不要**执行阻塞 I2C/SPI 操作
2. ISR 中**不要**执行耗时计算
3. ISR 只做"置标志位 + 简单 GPIO"
4. 主循环检测标志位再执行实际操作
5. `printf` 虽然在当前项目中 ISR 内可用（阻塞 UART），但不推荐

### 全局变量
- ISR 和主循环共享的标志位需加 `volatile`
- 单字节读写在 Cortex-M4 上是原子的，`uint8_t` 标志位不需要额外保护
- 复杂共享数据需考虑关中断保护或原子操作
