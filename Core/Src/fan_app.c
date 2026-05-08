#include "fan_app.h"

#include "drv8833.h"
#include "tim.h"

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
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
    __HAL_TIM_SET_COUNTER(&htim2, FAN_ENCODER_CENTER);
    fan_status.encoder_count = FAN_ENCODER_CENTER;
    fan_status.speed_percent = 0;
    fan_status.direction = FAN_DIRECTION_STOP;
}

void FAN_APP_Update(void)
{
    uint32_t raw_count = __HAL_TIM_GET_COUNTER(&htim2);

    if (raw_count > FAN_ENCODER_WRAP_THRESHOLD)
    {
        raw_count = 0;
        __HAL_TIM_SET_COUNTER(&htim2, 0);
    }

    if (raw_count > FAN_ENCODER_MAX)
    {
        raw_count = FAN_ENCODER_MAX;
        __HAL_TIM_SET_COUNTER(&htim2, raw_count);
    }

    fan_status.encoder_count = (uint16_t)raw_count;

    if (raw_count < FAN_ENCODER_CENTER)
    {
        fan_status.speed_percent = (uint8_t)((FAN_ENCODER_CENTER - raw_count) * 100U / FAN_ENCODER_CENTER);
        fan_status.direction = FAN_DIRECTION_BACKWARD;
        DRV8833_Backward(fan_status.speed_percent);
    }
    else if (raw_count > FAN_ENCODER_CENTER)
    {
        fan_status.speed_percent = (uint8_t)((raw_count - FAN_ENCODER_CENTER) * 100U / FAN_ENCODER_CENTER);
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