//
// Created by Moemeng on 2026/5/8.
//

#ifndef TEST_L432KC_DRV8833_H
#define TEST_L432KC_DRV8833_H
#include <stdint.h>

/**
 * @brief 衰减模式枚举
 */
typedef enum {
    SLOW_DECAY,  /**< 慢衰减模式 */
    FAST_DECAY   /**< 快衰减模式 */
} DecayMode;

/**
 * @brief 初始化DRV8833
 */
void DRV8833_Init(void);

/**
 * @brief 设置衰减模式
 * @param mode 衰减模式
 */
void DRV8833_SetDecayMode(DecayMode mode);

/**
 * @brief 控制电机前进
 * @param speed 速度值（0-100）
 */
void DRV8833_Forward(uint8_t speed);

/**
 * @brief 控制电机后退
 * @param speed 速度值（0-100）
 */
void DRV8833_Backward(uint8_t speed);

/**
 * @brief 电机刹车
 */
void DRV8833_Brake(void);

/**
 * @brief 电机滑行
 */
void DRV8833_Coast(void);

#endif //TEST_L432KC_DRV8833_H
