//
// Created by Moemeng on 2026/4/28.
//
#include "rtc_app.h"
#include "rtc.h"
#include<stdio.h>

static HAL_StatusTypeDef rtc_fill_date_time(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate,
                                            uint16_t year, uint8_t month, uint8_t day,
                                            uint8_t weekday, uint8_t hour, uint8_t minute, uint8_t second)
{
    if (year < 2000 || year > 2099 || month < 1 || month > 12 || day < 1 || day > 31 ||
        hour > 23 || minute > 59 || second > 59)
    {
        return HAL_ERROR;
    }

    sTime->Hours = hour;
    sTime->Minutes = minute;
    sTime->Seconds = second;
    sTime->DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime->StoreOperation = RTC_STOREOPERATION_RESET;

    sDate->Year = year - 2000;
    sDate->Month = month;
    sDate->Date = day;
    sDate->WeekDay = (weekday >= RTC_WEEKDAY_MONDAY && weekday <= RTC_WEEKDAY_SUNDAY) ? weekday : RTC_WEEKDAY_MONDAY;

    return HAL_OK;
}

void RTC_DATA_GET(RTC_TIME_DATA* data)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};


    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);//重要规则： HAL_RTC_GetDate() 和 HAL_RTC_GetTime() 必须一起调用，且先 Time 后Date，否则时间值会被锁定不更新（影子寄存器机制）。
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

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

    if (rtc_fill_date_time(&sTime, &sDate, year, month, day, weekday, hour, minute, second) != HAL_OK)
    {
        return HAL_ERROR;
    }

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


HAL_StatusTypeDef RTC_DATA_SET_BY_STRUCT(RTC_TIME_DATA* data)
{
    RTC_TimeTypeDef sTime={0};
    RTC_DateTypeDef sDate={0};

    if (rtc_fill_date_time(&sTime, &sDate, data->year, data->month, data->day, data->weekday,
                           data->hour, data->minute, data->second) != HAL_OK)
    {
        return HAL_ERROR;
    }

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
    RTC_TIME_DATA data = {0};
    int matched = sscanf(str, "%hu-%hhu-%hhu %hhu:%hhu:%hhu",
                         &data.year, &data.month, &data.day,
                         &data.hour, &data.minute, &data.second);
    if (matched != 6)
    {
        return HAL_ERROR;
    }
    return RTC_DATA_SET_BY_STRUCT(&data);
}