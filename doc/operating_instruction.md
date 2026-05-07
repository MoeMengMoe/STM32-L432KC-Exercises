# 操作说明

## 硬件接线

### OLED SSD1306 + AHT20 + BMP280
三个传感器共用 I2C3 总线（并联），接线如下：

| 模块引脚 | NUCLEO-L432KC 引脚 |
|---------|-------------------|
| VCC | CN3 pin 4 (3.3V) |
| GND | CN2 pin 16 或 CN3 pin 6 |
| SCL | CN2 pin 14 (D13 / PA7) |
| SDA | CN2 pin 2 (D0 / PB4) |

所有模块的 SCL 连一起、SDA 连一起、VCC 连一起、GND 连一起。

### 串口调试
| 信号 | 引脚 |
|------|------|
| TX (PA9) | USB 转串口 RX |
| RX (PA10) | USB 转串口 TX |
| GND | 共地 |

波特率: **115200**

### 按键与 LED
| 外设 | 引脚 | 说明 |
|------|------|------|
| KEY | PB5 | 按键 → GND，默认高电平 |
| LED_GREEN | PB6 | 低电平灭，高电平亮 |
| LED_RED | PB0/TEMP_ARM | 温度触发EXIT控制 |

---

## 操作方式

### 上电启动
1. 连接 USB 供电或 ST-Link
2. 串口助手连接（115200, 8N1）
3. 启动日志输出：
   ```
   Starting SSD1306 System...
   ssd1306: init success!
   aht20: init success!
   bmp280: init success!
   ```

### OLED 页面切换
- **开机默认**: TIME_PAGE — 显示日期和时间
- **按下 KEY (PB5)**: 切换到 STATUS_PAGE — 显示温湿度 + 气压
- **再按 KEY**: 切回 TIME_PAGE

### 设置 RTC 时间
通过串口发送以下格式数据：
```
2026-05-07-14:30:00
```
成功返回: `RTC updated successfully!`
失败返回: `Failed to update RTC. Please use format: YYYY-MM-DD-HH:MM:SS`

发送后 OLED 显示的新时间。

### 温度触发 LED
- 按住 TEMP_ARM 引脚附近的热源 → LED_RED 亮
- 移开 → LED_RED 灭
- 串口输出 `temp irq`

---

## OLED 显示内容

### TIME_PAGE
```
2026-05-07        (16px 字体, y=0)
14:30:00           (16px 字体, y=20)
```

### STATUS_PAGE
```
T:25.5C H:60%      (16px 字体, y=0)
P:1013.2hPa        (16px 字体, y=20)
```

---

## 构建与烧录

### 编译
```bash
# 首次需要先配置
cube-cmake --preset Debug

# 编译
cmake --build build/Debug
```

### 烧录
```bash
openocd -f board/st_nucleo_l432kc.cfg -c "program build/Debug/Test_L432KC.elf verify reset exit"
```

### 输出文件
- `build/Debug/Test_L432KC.elf`
- `build/Debug/Test_L432KC.hex`
- `build/Debug/Test_L432KC.bin`

### 调试
- 串口输出即调试日志，所有传感器状态和错误均通过 USART1 输出
- 编译数据库位于 `build/Debug/compile_commands.json`（供 clangd 使用）

---

## 常见问题

**Q: OLED 不亮**
A: 检查 I2C3 接线，确认 SCL/SDA 没有接反。上电后串口应有 `ssd1306: init failed` 输出。

**Q: 传感器显示 0 值**
A: 检查传感器 VCC 是否接 3.3V（不是 5V）。确认接线没有虚焊。

**Q: 编译错误 'undefined reference to ...'**
A: 运行 `cmake --build build/Debug` 前确保已执行 `cube-cmake --preset Debug`

**Q: BMP280 地址错误**
A: 确认模块上 ADO 引脚电平，调用 `bmp280_basic_init()` 时传入对应枚举值。
