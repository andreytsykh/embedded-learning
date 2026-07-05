#ifndef SAFE_CALLBACKS_H
#define SAFE_CALLBACKS_H

#include <stdint.h>
#include "esp_log.h"

#include "safe_indication.h"

void safe_unlock_action(void);
void safe_lock_action(void);
void safe_success_sound(void);
void safe_alarm_sound(void);
void safe_confirm_digit_cb(uint8_t digit_index, uint8_t digit_value);
void safe_digit_changed_cb(uint8_t digit_index, uint8_t digit_value);

#endif