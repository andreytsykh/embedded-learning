#ifndef SEVEN_SEGMENT_H
#define SEVEN_SEGMENT_H

#include <stdint.h>

#include "driver/gpio.h"
#include "esp_err.h"

typedef struct
{
    gpio_num_t segment_a_gpio;
    gpio_num_t segment_b_gpio;
    gpio_num_t segment_c_gpio;
    gpio_num_t segment_d_gpio;
    gpio_num_t segment_e_gpio;
    gpio_num_t segment_f_gpio;
    gpio_num_t segment_g_gpio;
} seven_segment_config_t;


esp_err_t seven_segment_init(const seven_segment_config_t *config);

esp_err_t seven_segment_show_digit(uint8_t digit);

esp_err_t seven_segment_clear(void);

#endif /* SEVEN_SEGMENT_H */