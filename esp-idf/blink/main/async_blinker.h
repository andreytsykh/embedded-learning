#pragma once

#include <stdint.h>
#include "driver/gpio.h"

typedef struct led_s led_t;

typedef void (*led_action_fn_t)(led_t *led);
typedef void (*led_update_fn_t)(led_t *led, uint32_t now_ms);

struct led_s {
    gpio_num_t pin;
    uint8_t mask;

    uint32_t interval_ms;
    uint32_t prev_ms;

    led_action_fn_t action;
    led_update_fn_t update;
};

void led_init(led_t *led);
void led_toggle(led_t *led);
void led_update_impl(led_t *led, uint32_t now_ms);