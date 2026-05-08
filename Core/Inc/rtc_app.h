//
// Created by Moemeng on 2026/4/28.
//

#ifndef TEST_L432KC_RTC_APP_H
#define TEST_L432KC_RTC_APP_H

#include "main.h"
#include "rtc.h"
#include <stdint.h>

typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t weekday;

    uint8_t hour;
    uint8_t minute;
    uint8_t second;
}RTC_TIME_DATA;

void RTC_DATA_GET(RTC_TIME_DATA *data);

HAL_StatusTypeDef RTC_DATA_SET(uint16_t year, uint8_t month, uint8_t day, uint8_t weekday, uint8_t hour, uint8_t minute, uint8_t second);

HAL_StatusTypeDef RTC_DATA_SET_BY_STRUCT(RTC_TIME_DATA *data);

HAL_StatusTypeDef RTC_DATA_SET_BY_STRING(char *str);


#endif //TEST_L432KC_RTC_APP_H
