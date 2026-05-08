/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include<stdio.h>
#include"driver_ssd1306_basic.h"
#include "driver_aht20_basic.h"
#include "driver_bmp280_basic.h"
#include "rtc_app.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TIME_PAGE 0
#define STATUS_PAGE 1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t message[20];
uint8_t rx_buf[64];
uint8_t rx_idx=0;
uint8_t rx_char;//建立环形缓冲区以解决串口收发被中断打断的情况
uint8_t CUR_PAGE = TIME_PAGE;
volatile uint8_t page_switch_flag = 0;
/*这个关键词意在告诉编译器不要优化这个变量 因为编译器无法识别由于中断等硬件操作导致的标志位变化
 * 通常用于修饰寄存器值 中断中的标志位
 */
RTC_TIME_DATA time;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void I2C3_Scan(void);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == KEY_Pin)
  {
    printf("key irq\r\n");
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
    CUR_PAGE=(CUR_PAGE+1)%2;
    page_switch_flag = 1;
  }
  // if (GPIO_Pin == TEMP_ARM_Pin)//温度高亮起小灯发送0 温度低小灯熄灭发送1 按住远离电位器和小灯的四块电阻可升温
  // {
  //   printf("temp irq\r\n");
  //   if (HAL_GPIO_ReadPin(TEMP_ARM_GPIO_Port, TEMP_ARM_Pin) == GPIO_PIN_RESET)
  //   {
  //     HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
  //   }
  //   else
  //   {
  //     HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
  //   }
  // }
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    if (rx_char=='\n' || rx_char=='\r')
    {
      rx_buf[rx_idx] = '\0'; // 终止字符串
      printf("Received: %s\r\n", rx_buf);
      if (RTC_DATA_SET_BY_STRING((char*)rx_buf)==HAL_OK)
      {
        printf("RTC updated successfully!\r\n");
      }
      else
      {
        printf("Failed to update RTC. Please use format: YYYY-MM-DD-HH:MM:SS\r\n");
      }
      rx_idx = 0; // 重置索引准备接收下一条命令
    }
    else
    {
      if (rx_idx < sizeof(rx_buf) - 1) // 确保不会溢出
      {
        rx_buf[rx_idx++] = rx_char; // 存储接收到的字符并递增索引
      }
      
    }
    HAL_UART_Receive_IT(&huart1, &rx_char, 1);
    // RTC_DATA_SET_BY_STRING((char*)message);
  }
}

int _write(int file, char *ptr, int len)
{

  HAL_StatusTypeDef status = HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);

  if (status == HAL_OK) {
    return len;
  } else {
    return 0;
  }
}
int _read(int file, char *ptr, int len)
{
  (void)file;
  (void)len;

  HAL_StatusTypeDef status = HAL_UART_Receive(&huart1, (uint8_t *)ptr, 1, HAL_MAX_DELAY);

  if (status == HAL_OK) {
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, 1, HAL_MAX_DELAY); // 回显
    return 1;
  } else {
    return 0; // EOF
  }
}

void SHOW_TIME_PAGE()
{
  /* 直接覆写，不清屏避免闪烁 */
  /* 第1行: 日期 */
  char buf[24];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d", time.year, time.month, time.day);
  ssd1306_basic_string(0, 0, buf, strlen(buf), 1, SSD1306_FONT_16);

  /* 第2行: 时间 */
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", time.hour, time.minute, time.second);
  ssd1306_basic_string(0, 20, buf, strlen(buf), 1, SSD1306_FONT_16);
  HAL_Delay(1000);
}

void SHOW_STATUS_PAGE(int aht20_temp_x10,uint8_t aht20_hum,int bmp280_press_hpa_x10)
{

  char buf[24];
  int td = aht20_temp_x10 % 10;
  if (td < 0) td = -td;
  snprintf(buf, sizeof(buf), "T:%d.%dC H:%d%%", aht20_temp_x10 / 10, td, aht20_hum);
  ssd1306_basic_string(0, 0, buf, strlen(buf), 1, SSD1306_FONT_16);

  int pd = bmp280_press_hpa_x10 % 10;
  if (pd < 0) pd = -pd;
  snprintf(buf, sizeof(buf), "P:%d.%dhPa", bmp280_press_hpa_x10 / 10, pd);
  ssd1306_basic_string(0, 20, buf, strlen(buf), 1, SSD1306_FONT_16);

  HAL_Delay(1000);
}


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_I2C3_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
  HAL_UART_Receive_IT(&huart1, &rx_char,1);
  uint8_t res;

  printf("Starting SSD1306 System...\r\n");

  res = ssd1306_basic_init(SSD1306_INTERFACE_IIC, SSD1306_ADDR_SA0_0);

  if (res != 0)
  {
    // 如果初始化失败，串口会打印具体原因
    printf("ssd1306: init failed, code: %d\r\n", res);
  }
  else
  {
    printf("ssd1306: init success!\r\n");

    ssd1306_basic_clear();

    ssd1306_basic_string(0, 0, "STM32L432KC", 11, 1, SSD1306_FONT_16);
    ssd1306_basic_string(0, 20, "LibDriver OK", 12, 1, SSD1306_FONT_12);

    ssd1306_basic_rect(0, 40, 127, 60, 1);

    HAL_Delay(1000);

    ssd1306_basic_clear();
  }

  /* 扫描 I2C3 总线，查看哪些设备在线 */
  I2C3_Scan();

  /* 初始化AHT20温湿度传感器 */
  res = aht20_basic_init();
  if (res != 0)
  {
    printf("aht20: init failed, code: %d\r\n", res);
  }
  else
  {
    printf("aht20: init success!\r\n");
  }

  /* 初始化BMP280气压传感器 */
  res = bmp280_basic_init(BMP280_INTERFACE_IIC, BMP280_ADDRESS_ADO_HIGH);
  if (res != 0)
  {
    printf("bmp280: init failed, code: %d\r\n", res);
  }
  else
  {
    printf("bmp280: init success!\r\n");
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    RTC_DATA_GET(&time);

    if (page_switch_flag)//避免在中断中处理清屏 拖慢速度 建议在中断中置标志位
    {
      ssd1306_basic_clear();
      page_switch_flag = 0;
    }

    switch (CUR_PAGE)
    {
    case TIME_PAGE:
      SHOW_TIME_PAGE();
      break;
    case STATUS_PAGE:
    {
        /*将传感器读取放到具体页状态中读取 避免全局读取拖慢显示进度*/
      float aht20_temp = 0.0f;
      uint8_t aht20_hum = 0;
      if (aht20_basic_read(&aht20_temp, &aht20_hum) != 0)
      {
        printf("aht20: read failed\r\n");
      }
      else
      {
        int t_x10 = (int)(aht20_temp * 10);
        printf("aht20: temp=%d.%d hum=%d\r\n", t_x10 / 10, (t_x10 < 0 ? -t_x10 : t_x10) % 10, aht20_hum);
      }

      float bmp280_temp = 0.0f;
      float bmp280_press = 0.0f;
      if (bmp280_basic_read(&bmp280_temp, &bmp280_press) != 0)
      {
        printf("bmp280: read failed\r\n");
      }
      else
      {
        int p_hpa_x10 = (int)(bmp280_press / 10.0f);
        printf("bmp280: press=%d.%d\r\n", p_hpa_x10 / 10, p_hpa_x10 % 10);
      }
      SHOW_STATUS_PAGE((int)(aht20_temp * 10), aht20_hum, (int)(bmp280_press / 10.0f));
      break;
    }
    default:
      CUR_PAGE=TIME_PAGE;
      break;

    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 36;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/* USER CODE BEGIN 4 */

/**
 * @brief  I2C 总线扫描：遍历所有 7-bit 地址，打印哪些设备有 ACK 响应
 * @param  hi2c: I2C 句柄指针
 */
static void I2C3_Scan(void)
{
  printf("I2C3 bus scan:\r\n");
  for (uint8_t addr = 1; addr < 128; addr++)
  {
    /* HAL_I2C_IsDevi·ceReady 使用 8-bit 地址（左移 1 位） */
    if (HAL_I2C_IsDeviceReady(&hi2c3, (uint16_t)(addr << 1), 1, 100) == HAL_OK)
    {
      printf("  0x%02X (7-bit) / 0x%02X (8-bit) -> ACK!\r\n", addr, addr << 1);
    }
  }
  printf("Scan done.\r\n");
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
