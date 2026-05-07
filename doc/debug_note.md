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
