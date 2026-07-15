#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#include "esp_err.h"

esp_err_t oled_display_init(void);
void oled_display_show_code(
    uint8_t index,
    uint8_t digit,
    uint8_t attempts_remaining
);
void oled_display_show_message(const char *message);

#endif /* DISPLAY_H */
