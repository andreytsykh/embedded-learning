#ifndef SAFE_H
#define SAFE_H

#include <stdint.h>

typedef enum {
    SAFE_STATE_ENTERING = 0,
    SAFE_STATE_OPEN,
    SAFE_STATE_LOCKED,
} safe_state_t;

typedef enum {
    SAFE_EVENT_ROTATE_CW = 0,
    SAFE_EVENT_ROTATE_CCW,
    SAFE_EVENT_RESET,
} safe_event_t;

typedef enum {
    CW,
    CCW,
    NONE
} safe_direction_t;

typedef void (*safe_digit_changed_cb_t)(uint8_t digit_index, uint8_t digit_value);

typedef struct {
    safe_digit_changed_cb_t on_digit_changed;
} safe_config_t;

void         safe_init(const safe_config_t *config);
void         safe_process_event(safe_event_t event);
safe_state_t safe_get_state(void);
uint8_t      safe_get_attempts_used(void);

#endif