//
// Created by Moemeng on 2026/4/28.
//
#include "rtc_app.h"
#include "rtc.h"
#include<stdio.h>

void RTC_DATA_GET(RTC_TIME_DATA* data)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    data->year = sDate.Year+2000;//we use the year num after 2000
    data->month=sDate.Month;
    data->day=sDate.Date;
    data->weekday=sDate.WeekDay;
    data->hour=sTime.Hours;
    data->minute=sTime.Minutes;
    data->second=sTime.Seconds;
}
HAL_StatusTypeDef RTC_DATA_SET(uint16_t year, uint8_t month, uint8_t day, uint8_t weekday, uint8_t hour, uint8_t minute, uint8_t second)
{
    RTC_TimeTypeDef sTime={0};
    RTC_DateTypeDef sDate={0};

    sTime.Hours=hour;
    sTime.Minutes=minute;
    sTime.Seconds=second;

    sDate.Year=year;
    sDate.Month=month;
    sDate.Date=day;
    sDate.WeekDay=weekday;
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        return HAL_ERROR;
    }
    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        return HAL_ERROR;
    }
    return HAL_OK;

}


HAL_StatusTypeDef RTC_DATA_SET(RTC_TIME_DATA* data)
{
    RTC_TimeTypeDef sTime={0};
    RTC_DateTypeDef sDate={0};
    sTime.Hours=data->hour;
    sTime.Minutes=data->minute;
    sTime.Seconds=data->second;

    sDate.Year=data->year;
    sDate.Month=data->month;
    sDate.Date=data->day;
    sDate.WeekDay=data->weekday;

    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        return HAL_ERROR;
    }
    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        return HAL_ERROR;
    }
    return HAL_OK;

}

HAL_StatusTypeDef RTC_DATA_SET_BY_STRING(char* str)
{
    RTC_TIME_DATA data;
    sscanf(str, "%hu-%hhu-%hhu %hhu:%hhu:%hhu", &data.year, &data.month, &data.day, &data.hour, &data.minute, &data.second);
    return RTC_DATA_SET(&data);
}