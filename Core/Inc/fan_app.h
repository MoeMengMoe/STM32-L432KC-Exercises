#ifndef TEST_L432KC_FAN_APP_H
#define TEST_L432KC_FAN_APP_H

#include <stdint.h>
extern int tempx10;
extern uint16_t temp_thr;
typedef enum {
    FAN_DIRECTION_STOP = 0,
    FAN_DIRECTION_FORWARD,
    FAN_DIRECTION_BACKWARD
} FanDirection;



typedef struct {
    uint8_t speed_percent;
    FanDirection direction;
    uint16_t encoder_count;
} FanStatus;

void FAN_APP_Init(void);
void FAN_APP_Update(void);
FanStatus FAN_APP_GetStatus(void);
void FAN_Auto_Mode(int tempx10,int temp_thr);

#endif /* TEST_L432KC_FAN_APP_H */
