#ifndef __BUZZER_APP_H
#define __BUZZER_APP_H

#include "main.h"

// 音符频率定义 (Hz)
#define NOTE_REST 0
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880

typedef struct {
    uint16_t frequency;
    uint32_t duration_ms;
} BuzzerNote;

/**
 * @brief 初始化蜂鸣器模块
 * @param htim 定时器句柄
 * @param channel 定时器通道
 */
void BUZZER_Init(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief 开始播放乐谱
 * @param melody 乐谱数组
 * @param length 乐谱长度
 */
void BUZZER_PlayMelody(const BuzzerNote *melody, uint16_t length);

/**
 * @brief 蜂鸣器状态更新（需放在主循环调用，非阻塞）
 */
void BUZZER_Update(void);

#endif
