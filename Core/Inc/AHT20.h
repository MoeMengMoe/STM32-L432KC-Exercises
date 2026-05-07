//
// Created by Moemeng on 2026/5/7.
//

#ifndef TEST_L432KC_AHT20_H
#define TEST_L432KC_AHT20_H

#include "i2c.h"
#include "main.h"
void AHT20_Init();

// 获取温度和湿度
void AHT20_Read(float *Temperature, float *Humidity);
#endif //TEST_L432KC_AHT20_H
