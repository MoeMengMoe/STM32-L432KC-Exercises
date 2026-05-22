#include "fan_app.h"

#include "drv8833.h"
#include "tim.h"
#include <stdio.h>

#define FAN_ENCODER_CENTER 20U
#define FAN_ENCODER_MAX (FAN_ENCODER_CENTER * 2U)
#define FAN_ENCODER_WRAP_THRESHOLD 60000U

static FanStatus fan_status = {
    .speed_percent = 0,
    .direction = FAN_DIRECTION_STOP,
    .encoder_count = FAN_ENCODER_CENTER
};

void FAN_APP_Init(void)
{
    DRV8833_Init();
    if (HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL) != HAL_OK)
    {
        printf("TIM2 encoder start failed\r\n");
    }
    __HAL_TIM_SET_COUNTER(&htim2, FAN_ENCODER_CENTER);
    fan_status.encoder_count = FAN_ENCODER_CENTER;
    fan_status.speed_percent = 0;
    fan_status.direction = FAN_DIRECTION_STOP;
}

void FAN_APP_DebugEncoder(void)
{
    static uint16_t last_report_cnt = FAN_ENCODER_CENTER;
    static uint32_t last_periodic_tick = 0;
    uint16_t current_cnt = (uint16_t)__HAL_TIM_GET_COUNTER(&htim2);
    uint32_t now = HAL_GetTick();

    if ((current_cnt != last_report_cnt) || ((now - last_periodic_tick) >= 1000U))
    {
        printf("enc cnt=%u delta=%d pa0=%u pa1=%u pa5=%u\r\n",
               current_cnt,
               (int16_t)(current_cnt - last_report_cnt),
               (unsigned int)HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0),
               (unsigned int)HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1),
               (unsigned int)HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5));
        last_report_cnt = current_cnt;
        last_periodic_tick = now;
    }
}

void FAN_APP_Update(void)
{
    static uint16_t last_cnt = FAN_ENCODER_CENTER;
    static int16_t soft_encoder_val = FAN_ENCODER_CENTER;

    uint16_t current_cnt = (uint16_t)__HAL_TIM_GET_COUNTER(&htim2);
    int16_t delta = (int16_t)(current_cnt - last_cnt);
    last_cnt = current_cnt;

    if (delta != 0)
    {
        soft_encoder_val += delta;
        if (soft_encoder_val < 0)
        {
            soft_encoder_val = 0;
        }
        else if (soft_encoder_val > (int16_t)FAN_ENCODER_MAX)
        {
            soft_encoder_val = (int16_t)FAN_ENCODER_MAX;
        }
    }

    fan_status.encoder_count = (uint16_t)soft_encoder_val;

    if (soft_encoder_val < FAN_ENCODER_CENTER)
    {
        fan_status.speed_percent = (uint8_t)((FAN_ENCODER_CENTER - soft_encoder_val) * 100U / FAN_ENCODER_CENTER);
        fan_status.direction = FAN_DIRECTION_BACKWARD;
        DRV8833_Backward(fan_status.speed_percent);
    }
    else if (soft_encoder_val > FAN_ENCODER_CENTER)
    {
        fan_status.speed_percent = (uint8_t)((soft_encoder_val - FAN_ENCODER_CENTER) * 100U / FAN_ENCODER_CENTER);
        fan_status.direction = FAN_DIRECTION_FORWARD;
        DRV8833_Forward(fan_status.speed_percent);
    }
    else
    {
        fan_status.speed_percent = 0;
        fan_status.direction = FAN_DIRECTION_STOP;
        DRV8833_Coast();
    }
}

FanStatus FAN_APP_GetStatus(void)
{
    return fan_status;
}

void FAN_Auto_Mode(int tempx10, int temp_thr){
    int diff=tempx10-temp_thr;
    fan_status.direction=FAN_DIRECTION_FORWARD;
    if(diff<=0){
        DRV8833_Forward(20);
        fan_status.speed_percent=20;
    }
    else if(diff>0&&diff<=20){
        DRV8833_Forward(20);
        fan_status.speed_percent=20;
    }
    else if(diff>20&&diff<=30){
        fan_status.direction=FAN_DIRECTION_STOP;
        DRV8833_Brake();
        fan_status.speed_percent=0;
    }
    else if(diff>30&&diff<=40){
        DRV8833_Forward(40);
        fan_status.speed_percent=40;
    }
    else if(diff>40&&diff<=60){
        DRV8833_Forward(60);
        fan_status.speed_percent=60;
    }
    else if(diff>60){
        DRV8833_Forward(80);
        fan_status.speed_percent=80;
    }
    
    
}
