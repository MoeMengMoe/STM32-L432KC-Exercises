#include "buzzer_app.h"
#include "tim.h"

static TIM_HandleTypeDef *pBuzzerTim;
static uint32_t buzzerChannel;

static const BuzzerNote *pCurrentMelody = NULL;
static uint16_t melodyLength = 0;
static uint16_t currentNoteIdx = 0;
static uint32_t noteStartTime = 0;
static uint8_t isPlaying = 0;

void BUZZER_Init(TIM_HandleTypeDef *htim, uint32_t channel) {
    pBuzzerTim = htim;
    buzzerChannel = channel;
    isPlaying = 0;
}

static void BUZZER_SetFrequency(uint16_t freq) {
    if (freq == 0) {
        HAL_TIM_PWM_Stop(pBuzzerTim, buzzerChannel);
        return;
    }

    // 计算 ARR: F_pwm = F_timer / ((PSC+1) * (ARR+1))
    // 假设 PSC 已经设置为 71 (即 1MHz 计数频率)
    uint32_t timer_clock = 72000000; // 72MHz
    uint32_t psc = pBuzzerTim->Instance->PSC;
    uint32_t arr = (timer_clock / (psc + 1)) / freq - 1;

    __HAL_TIM_SET_AUTORELOAD(pBuzzerTim, arr);
    __HAL_TIM_SET_COMPARE(pBuzzerTim, buzzerChannel, arr / 2); // 50% 占空比
    
    HAL_TIM_PWM_Start(pBuzzerTim, buzzerChannel);
}

void BUZZER_PlayMelody(const BuzzerNote *melody, uint16_t length) {
    pCurrentMelody = melody;
    melodyLength = length;
    currentNoteIdx = 0;
    noteStartTime = HAL_GetTick();
    isPlaying = 1;
    BUZZER_SetFrequency(pCurrentMelody[0].frequency);
}

void BUZZER_Update(void) {
    if (!isPlaying || pCurrentMelody == NULL) return;

    uint32_t now = HAL_GetTick();
    if (now - noteStartTime >= pCurrentMelody[currentNoteIdx].duration_ms) {
        currentNoteIdx++;
        
        if (currentNoteIdx < melodyLength) {
            noteStartTime = now;
            BUZZER_SetFrequency(pCurrentMelody[currentNoteIdx].frequency);
        } else {
            // 播放结束
            isPlaying = 0;
            BUZZER_SetFrequency(0);
        }
    }
}
