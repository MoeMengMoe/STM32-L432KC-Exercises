# 踩坑记录 / Debug Notes

## 1. RTC 影子寄存器陷阱

**现象**: 连续读取 RTC 时间时，时间值不更新（卡在同一个值）。

**原因**: STM32L4 的 RTC 有影子寄存器机制。`HAL_RTC_GetTime()` 和 `HAL_RTC_GetDate()` 必须**成对调用**，且**先调 GetTime 再调 GetDate**，否则影子寄存器不会刷新，返回的是上次缓存的值。

**解决**: 
```c
HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);  // 必须先 Time
HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);  // 再 Date
```

## 2. ISR 中执行 I2C 操作

**现象**: 按键按下时触发 I2C 清屏，屏幕闪烁或 I2C 卡死。

**原因**: I2C 是阻塞式多字节传输，在中断上下文（HAL_GPIO_EXTI_Callback）中调用 HAL_I2C 相关函数，可能和主循环中的 I2C 操作冲突。且 `printf` 本身也在 ISR 中做 UART 传输。

**解决**: ISR 只做"置标志位"，I2C/耗时操作放在主循环中处理：
```c
// ISR 中
page_switch_flag = 1;

// 主循环中
if (page_switch_flag) {
    ssd1306_basic_clear();
    page_switch_flag = 0;
}
```

ISR 中应遵循**快进快出**原则，避免复杂外设操作。

## 3. OLED 闪屏

**现象**: 主循环中调用 `ssd1306_basic_clear()` 后逐个绘制文字，屏幕明显闪烁。

**原因**: 清屏后所有像素置黑，再逐行写入新内容。每次 I2C 写操作耗时较长，中间出现短暂全黑 → 可见闪烁。

**解决**: 不再清屏，直接覆写相同位置的文字：
```c
// ❌ 错误：会闪
ssd1306_basic_clear();
ssd1306_basic_string(0, 0, "...");  

// ✅ 正确：直接覆写
ssd1306_basic_string(0, 0, "...");  
```
前提是每次写入字符长度固定（或覆盖足够宽的区域）。

## 4. printf 与串口中断

**现象**: `printf` 在 ISR 中使用时偶尔丢失数据或卡死。

**原因**: `printf` 最终调用 `_write()`，使用 `HAL_UART_Transmit` 阻塞发送。在中断中做阻塞发送可能导致中断嵌套或优先级反转。

**注意**: `printf` 在 ISR 中确实不推荐，但 USART1 的 `HAL_UART_Transmit` 在当前配置下工作正常。如果后续添加更复杂的中断嵌套场景，需考虑使用中断发送+DMA或者环形缓冲区。

## 5. `snprintf` 缓冲区不足

**现象**: 编译警告 `warning: 'snprintf' output between X and Y bytes into a destination of size Z`。

**原因**: 格式串加上数据后实际长度可能超过分配的缓冲区大小。例如：
```c
char buf[20];
snprintf(buf, 20, "T:%0.1fC H:%d%% P:%0.1fhPa", ...);  // 实际需要 >20 字节
```

**解决**: 确保 `buf` 大小足够容纳完整格式串的最大可能输出。常用格式：
- `"%04d-%02d-%02d"` → 11 字符 → buf[16] 安全
- `"%02d:%02d:%02d"` → 9 字符 → buf[12] 安全
- `"T:%0.1fC H:%d%%"` → ~14 字符 → buf[20] 安全
- `"P:%0.1fhPa"` → ~13 字符 → buf[20] 安全

## 6. I2C 地址格式

**现象**: I2C 通信无响应或收到 NACK。

**原因**: HAL 库的 I2C 地址使用 **8-bit 地址**（7-bit 地址左移 1 位），而数据手册通常给出 7-bit 地址。

**对照表**:
| 设备 | 7-bit | 8-bit (HAL 用) |
|------|-------|---------------|
| SSD1306 | 0x3C | 0x78 |
| AHT20 | 0x38 | 0x70 |
| BMP280 (ADO=LOW) | 0x76 | 0xEC |
| BMP280 (ADO=HIGH) | 0x77 | 0xEE |

## 7. volatile 关键字

**现象**: 中断中修改的变量在主循环中检测不到变化。

**原因**: 编译器优化将变量缓存到寄存器，ISR 修改变量后主循环读到的仍是寄存器旧值。

**解决**: 在 ISR 和主循环共享的标志位变量前加 `volatile`：
```c
volatile uint8_t page_switch_flag = 0;
```
**注意**: `volatile` 不保证原子性。Cortex-M4 的单字节读写是原子的，所以 `uint8_t` 标志位不需要额外保护。

## 8. BMP280 ADO 引脚地址

BMP280 的 I2C 地址由 SDO/ADO 引脚电平决定：
- ADO = LOW (接 GND): 地址 0x76 (7-bit)
- ADO = HIGH (接 3.3V): 地址 0x77 (7-bit)

初始化时需要传入正确的参数：
```c
// ADO 接 GND
bmp280_basic_init(BMP280_INTERFACE_IIC, BMP280_ADDRESS_ADO_LOW);
// ADO 接 3.3V
bmp280_basic_init(BMP280_INTERFACE_IIC, BMP280_ADDRESS_ADO_HIGH);
```

## 9. I2C 时钟过快导致传感器无响应

**现象**: SSD1306 偶尔工作，AHT20/BMP280 完全无响应，串口输出 "read status failed" / "read id failed"。但 I2C 总线扫描（`HAL_I2C_IsDeviceReady`）能 ACK。

**原因**: CubeMX 默认生成的 I2C3 Timing `0x00A10D1F` 在 PCLK1=75.5MHz 下计算出 SCL ≈ 1.7MHz，远超传感器支持的 400kHz（标准 Fast Mode）。传感器能响应地址（ACK 地址字节），但多字节数据传输时失败。

**计算方式**: STM32L4 I2C TIMINGR 寄存器：
```
[31:28] PRESC   = 定时器预分频
[27:24] SCLDEL  = 数据建立时间
[23:20] SDADEL  = 数据保持时间
[15:8]  SCLH    = SCL 高电平周期
[7:0]   SCLL    = SCL 低电平周期
```
SCL 频率公式：`f_SCL = PCLK1 / ((SCLH + SCLL) × (PRESC + 1))`

**解决**: 在 `USER CODE BEGIN I2C3_Init 2` 中覆盖 Timing 值，降速到 ~100kHz：
```c
hi2c3.Init.Timing = 0x36905E5F;  // PRESC=3, SCLH=94, SCLL=95 → 100kHz
```
不用修改 CubeMX 生成的代码，在 USER CODE 区域覆盖即可。

## 10. BMP280 地址选错（COMBO 模块）

**现象**: BMP280 初始化失败 "read id failed"。I2C 扫描显示 BMP280 在 0x77，但代码传了 `BMP280_ADDRESS_ADO_LOW`（0x76）。

**原因**: Combo 模块（AHT20+BMP280 一体）上 BMP280 的 ADO 引脚被拉高到 VCC，地址为 0x77。

**解决**: 改用 `BMP280_ADDRESS_ADO_HIGH`：
```c
bmp280_basic_init(BMP280_INTERFACE_IIC, BMP280_ADDRESS_ADO_HIGH);
```
**教训**: 仅看 I2C 扫描结果比猜地址更可靠。怀疑地址问题时先扫总线。

## 11. newlib-nano 浮点数格式化裁剪

**现象**: `printf` 和 `snprintf` 中用 `%f` 输出空字符串（`temp=`），但 `%d` 正常。

**原因**: 工具链使用 `--specs=nano.specs`（newlib-nano），为节省 Flash 默认不链接 `printf`/`snprintf` 的浮点数格式化代码。

**两种解决方案**:

| 方案 | 修改 | Flash 开销 | 优点 |
|------|------|-----------|------|
| 链接浮点支持 | CMake 加 `-u _printf_float` | +~10KB | 代码简洁，直接 `%.1f` |
| 手动整数转换 | 传感器值 ×10，用 `%d.%d` 输出 | 无 | 不占额外 Flash |

**选择**: 本项目 Flash 余量充足（256KB 用 ~57KB），但选择整数转换方案以避免将来无意引入浮点格式化导致 Flash 膨胀。

**示例**:
```c
// ❌ 需要浮点支持
snprintf(buf, 20, "T:%0.1fC", temp);

// ✅ 无浮点依赖
int t_x10 = (int)(temp * 10);
snprintf(buf, 20, "T:%d.%dC", t_x10 / 10, abs(t_x10 % 10));
```

## 12. HAL_GetTick 代替 HAL_Delay 做非阻塞延时

**教训**: 不要在主循环或 ISR 中使用 `HAL_Delay()`，它会阻塞整个 MCU 执行。需要计时的场景用 `HAL_GetTick()` + 时间差判断。

**问题**: `HAL_Delay(1000)` 放在显示函数中导致每页刷新阻塞 1 秒，风扇控制更新频率也被拖慢到 1Hz，编码器响应迟缓。SHOW 函数应该只负责绘制，延时交给主循环统一管理。

**解决模式**:
```c
static uint32_t last_update = 0;

void main_loop(void) {
    uint32_t now = HAL_GetTick();
    if (now - last_update >= 100) {   // 100ms 刷新周期
        last_update = now;
        // 执行显示刷新
    }
    // 其他代码不阻塞，持续运行
}
```

**对比**:
| 方式 | 行为 | 适用场景 |
|------|------|---------|
| `HAL_Delay(n)` | 死等 n ms，期间不执行任何代码 | 初始化阶段一次性操作 |
| `HAL_GetTick()` + 时间差 | 不阻塞，其他代码继续执行 | 主循环周期性任务 |

## 13. 多用 printf 串口输出调试信息

**经验**: STM32 嵌入式开发没有硬件调试器时，`printf` 是最可靠、最快速的调试手段。关键位置打日志能迅速区分硬件问题还是软件问题。

**实践**:
- 初始化每一步后打印结果：`printf("ssd1306: init success!\r\n");`
- EXTI 回调入口打印触发源：`printf("key irq\r\n");`
- 传感器读取后打印原始值：`printf("aht20: temp=%d.%d hum=%d\r\n", ...);`
- I2C 总线扫描打印所有 ACK 地址：`printf("  0x%02X -> ACK!\r\n", addr);`

这些日志在串口终端（115200 bps）可实时查看。与 HAL_Delay 不同，printf 在调试阶段的耗时是"必要的时间开销"而非负担。

## 14. EXTI 边沿配置错误导致上电误触发

**现象**: 上电/初始化后，编码器按键（ENC_KEY, PA6）在第一轮主循环就不期望地触发了一次模式切换（MANUAL ↔ AUTO）。

**原因**: ENC_KEY 在 CubeMX 中配置为 `GPIO_MODE_IT_RISING`（上升沿触发）+ `GPIO_PULLUP`（内部上拉），犯了一个基本错误——**上升沿触发配了上拉电阻**。上电后引脚被上拉到高电平，但初始化阶段 GPIO 模式切换时引脚电平短暂跳变，产生了伪上升沿，误触发 EXTI。

**解决**: 将 ENC_KEY 改为 `GPIO_MODE_IT_FALLING`（下降沿触发）+ `GPIO_PULLUP`（内部上拉）。按键按下时引脚被拉到低电平 → 产生下降沿 → 正常触发。上电后引脚稳定在高电平，不会产生下降沿。

**教训**:
1. **按键/开关类**输入：**下降沿触发 + 上拉** 是标准组合（按下=低电平=下降沿）
2. **上升沿触发 + 下拉** 适合开漏输出传感器（默认低，输出时拉高）
3. 两个 EXTI 共用同一个 IRQ（EXTI9_5）时，在回调中打印 `GPIO_Pin` 区分触发源
4. 不要混搭：上拉配上升沿、下拉配下降沿都是错误配置
