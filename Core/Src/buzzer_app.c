#include "buzzer_app.h"
#include "tim.h"

static TIM_HandleTypeDef *pBuzzerTim;
static uint32_t buzzerChannel;

static const BuzzerNote *pCurrentMelody = NULL;
static uint16_t melodyLength = 0;
static uint16_t currentNoteIdx = 0;
static uint32_t noteStartTime = 0;
static uint8_t isPlaying = 0;
static uint8_t noteGapActive = 0;

#define BUZZER_DUTY_PERCENT 30U
#define BUZZER_NOTE_GAP_MS  20U

void BUZZER_Init(TIM_HandleTypeDef *htim, uint32_t channel) {
    pBuzzerTim = htim;
    buzzerChannel = channel;
    isPlaying = 0;
    noteGapActive = 0;
}

static uint32_t BUZZER_GetTimerClock(void) {
    if (pBuzzerTim == NULL || pBuzzerTim->Instance == NULL) {
        return 0;
    }

    if (pBuzzerTim->Instance == TIM15) {
        uint32_t pclk = HAL_RCC_GetPCLK2Freq();
        return ((RCC->CFGR & RCC_CFGR_PPRE2) == RCC_HCLK_DIV1) ? pclk : (pclk * 2U);
    }

    return HAL_RCC_GetPCLK1Freq();
}

static void BUZZER_Stop(void) {
    if (pBuzzerTim == NULL) {
        return;
    }

    HAL_TIM_PWM_Stop(pBuzzerTim, buzzerChannel);
    __HAL_TIM_SET_COMPARE(pBuzzerTim, buzzerChannel, 0);
}

static void BUZZER_SetFrequency(uint16_t freq) {
    if (pBuzzerTim == NULL) {
        return;
    }

    if (freq == 0) {
        BUZZER_Stop();
        return;
    }

    uint32_t timer_clock = BUZZER_GetTimerClock();
    uint32_t psc = pBuzzerTim->Instance->PSC;
    if (timer_clock == 0U || psc == UINT32_MAX) {
        BUZZER_Stop();
        return;
    }

    uint32_t arr = (timer_clock / (psc + 1)) / freq - 1;
    if (arr < 2U || arr > 0xFFFFU) {
        BUZZER_Stop();
        return;
    }

    HAL_TIM_PWM_Stop(pBuzzerTim, buzzerChannel);
    __HAL_TIM_SET_COUNTER(pBuzzerTim, 0);
    __HAL_TIM_SET_AUTORELOAD(pBuzzerTim, arr);
    __HAL_TIM_SET_COMPARE(pBuzzerTim, buzzerChannel, ((arr + 1U) * BUZZER_DUTY_PERCENT) / 100U);
    HAL_TIM_GenerateEvent(pBuzzerTim, TIM_EVENTSOURCE_UPDATE);
    HAL_TIM_PWM_Start(pBuzzerTim, buzzerChannel);
}

void BUZZER_PlayMelody(const BuzzerNote *melody, uint16_t length) {
    if (melody == NULL || length == 0U) {
        pCurrentMelody = NULL;
        melodyLength = 0;
        currentNoteIdx = 0;
        isPlaying = 0;
        noteGapActive = 0;
        BUZZER_Stop();
        return;
    }

    pCurrentMelody = melody;
    melodyLength = length;
    currentNoteIdx = 0;
    noteStartTime = HAL_GetTick();
    isPlaying = 1;
    noteGapActive = 0;
    BUZZER_SetFrequency(pCurrentMelody[0].frequency);
}

void BUZZER_Update(void) {
    if (!isPlaying || pCurrentMelody == NULL) return;

    uint32_t now = HAL_GetTick();
    if (noteGapActive) {
        if ((now - noteStartTime) >= BUZZER_NOTE_GAP_MS) {
            noteGapActive = 0;
            noteStartTime = now;
            BUZZER_SetFrequency(pCurrentMelody[currentNoteIdx].frequency);
        }
        return;
    }

    if (now - noteStartTime >= pCurrentMelody[currentNoteIdx].duration_ms) {
        currentNoteIdx++;
        
        if (currentNoteIdx < melodyLength) {
            noteStartTime = now;
            noteGapActive = 1;
            BUZZER_Stop();
        } else {
            // 播放结束
            isPlaying = 0;
            noteGapActive = 0;
            BUZZER_Stop();
        }
    }
}
