#pragma once

#include "driver/gpio.h"
#include "esp_timer.h"

typedef enum {
    BUTTON_IDLE,
    BUTTON_PRESSING,
    BUTTON_SHORT_PRESS,
    BUTTON_LONG_PRESS
} button_state_t;

typedef void (*button_cb_t)(void *ctx);

typedef struct {
    gpio_num_t gpio;
    button_state_t state;

    esp_timer_handle_t debounce_timer;
    esp_timer_handle_t long_press_timer;

    uint64_t debounce_us;
    uint64_t long_press_us;

    button_cb_t on_short_press;
    button_cb_t on_long_press;
    void *ctx;
} button_t;

void button_init(button_t *btn);

static inline button_state_t button_get_state(const button_t *btn)
{
    return btn->state;
}