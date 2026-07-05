#ifndef SAFE_INDICATION_H
#define SAFE_INDICATION_H

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_timer.h"

#include "seven_segment.h"

typedef struct
{
    gpio_num_t gpio_bit_0;
    gpio_num_t gpio_bit_1;
    gpio_num_t gpio_bit_2; 
    gpio_num_t gpio_bit_3;
    gpio_num_t gpio_lock;
    gpio_num_t gpio_unlock;

    bool active_high;
} safe_indication_config_t;

typedef enum
{
    SAFE_INDICATION_LED_OFF = 0,
    SAFE_INDICATION_LED_ON,
    SAFE_INDICATION_LED_BLINK
} safe_indication_led_mode_t;

esp_err_t safe_indication_set_led_mode(
    uint8_t index,
    safe_indication_led_mode_t mode);


esp_err_t safe_indication_init(
    const safe_indication_config_t *config);

esp_err_t safe_indication_off(void);

esp_err_t safe_indication_on(void);

esp_err_t safe_indication_show_digit(uint8_t);

esp_err_t safe_indication_activate_led(uint8_t index);

esp_err_t safe_indication_blink_led(uint8_t index);

bool safe_indication_is_enabled(void);

#endif /* SAFE_INDICATION_H */