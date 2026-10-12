#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "driver/gpio.h"

typedef struct led_s led_t;

typedef void (*led_action_fn_t)(led_t *led);

struct led_s {
    gpio_num_t pin;
    uint32_t interval_ms;
    bool state;

    led_action_fn_t action;
};

void led_init(led_t *led);
void led_toggle(led_t *led);
void led_task(void *arg);