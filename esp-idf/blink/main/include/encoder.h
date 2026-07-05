#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

#include "driver/gpio.h"
#include "esp_err.h"

typedef enum
{
    ENCODER_EVENT_NONE = 0,
    ENCODER_EVENT_CW,
    ENCODER_EVENT_CCW
} encoder_event_t;

typedef struct
{
    gpio_num_t gpio_a;
    gpio_num_t gpio_b;

    int32_t counts_per_detent;
    uint32_t glitch_filter_ns;
} encoder_config_t;

esp_err_t encoder_init(const encoder_config_t *config);

encoder_event_t encoder_get_event(void);

esp_err_t encoder_reset(void);

#endif