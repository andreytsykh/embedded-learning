#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "button.h"
#include "traffic_light.h"
#include "config.h"

static volatile bool start_traffic_light_requested = false;
static volatile bool yellow_blink_requested = false;

static void on_button_short_press(void *ctx)
{
    start_traffic_light_requested = true;
}

static void on_button_long_press(void *ctx)
{
    yellow_blink_requested = true;
}

static button_t button = {
    .gpio = BUTTON_GPIO,
    .state = BUTTON_IDLE,

    .debounce_us = BUTTON_DEBOUNCE_US,
    .long_press_us = BUTTON_LONG_PRESS_US,

    .on_short_press = on_button_short_press,
    .on_long_press = on_button_long_press,
    .ctx = NULL};

void app_main(void)
{
    traffic_light_init();
    button_init(&button);

    while (1)
    {
        if (start_traffic_light_requested)
        {
            start_traffic_light_requested = false;
            traffic_light_start();
        }
        if (yellow_blink_requested)
        {
            yellow_blink_requested = false;
            traffic_light_toggle_yellow_blink();
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
