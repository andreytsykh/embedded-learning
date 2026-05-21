#include <stddef.h>

#include "esp_timer.h"
#include "esp_check.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "async_blinker.h"

#define LED_1 GPIO_NUM_4
#define LED_2 GPIO_NUM_5
#define LED_3 GPIO_NUM_18

#define LED1_MASK (1u << 0)
#define LED2_MASK (1u << 1)
#define LED3_MASK (1u << 2)

static uint8_t led_state = 0;

static uint32_t millis(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

void led_init(led_t *led)
{
    gpio_reset_pin(led->pin);
    gpio_set_direction(
        led->pin,
        GPIO_MODE_OUTPUT);
    gpio_set_level(led->pin, 0);
}

void led_toggle(led_t *led)
{
    led_state ^= led->mask;
    gpio_set_level(
        led->pin,
        (led_state & led->mask) != 0);
}

void led_update_impl(
    led_t *led,
    uint32_t now_ms)
{

    if ((now_ms - led->prev_ms) >= led->interval_ms)
    {

        led->prev_ms += led->interval_ms;

        led->action(led);
    }
}

void app_main(void)
{

    led_t leds[] = {
        {.pin = LED_1,
         .mask = LED1_MASK,
         .interval_ms = 500,
         .action = led_toggle,
         .update = led_update_impl},
        {.pin = LED_2,
         .mask = LED2_MASK,
         .interval_ms = 700,
         .action = led_toggle,
         .update = led_update_impl},
        {.pin = LED_3,
         .mask = LED3_MASK,
         .interval_ms = 1100,
         .action = led_toggle,
         .update = led_update_impl}};

    size_t led_count =
        sizeof(leds) / sizeof(leds[0]);

    uint32_t start_ms = millis();

    for (size_t i = 0; i < led_count; i++)
    {

        leds[i].prev_ms = start_ms;

        led_init(&leds[i]);
    }

    while (1)
    {

        uint32_t now_ms = millis();

        for (size_t i = 0; i < led_count; i++)
        {

            leds[i].update(
                &leds[i],
                now_ms);
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}