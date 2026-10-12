#include <stddef.h>

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "async_blinker.h"

#define LED_1 GPIO_NUM_4
#define LED_2 GPIO_NUM_5
#define LED_3 GPIO_NUM_18

#define LED_COUNT 3
#define LED_TASK_STACK_SIZE 2048
#define LED_TASK_PRIORITY 5

static led_t leds[LED_COUNT] = {
    {
        .pin = LED_1,
        .interval_ms = 500,
        .state = false,
        .action = led_toggle
    },
    {
        .pin = LED_2,
        .interval_ms = 700,
        .state = false,
        .action = led_toggle
    },
    {
        .pin = LED_3,
        .interval_ms = 1100,
        .state = false,
        .action = led_toggle
    }
};

void led_init(led_t *led)
{
    ESP_ERROR_CHECK(gpio_reset_pin(led->pin));

    ESP_ERROR_CHECK(gpio_set_direction(
        led->pin,
        GPIO_MODE_OUTPUT
    ));

    ESP_ERROR_CHECK(gpio_set_level(led->pin, 0));
}

void led_toggle(led_t *led)
{
    led->state = !led->state;

    gpio_set_level(
        led->pin,
        led->state
    );
}

void led_task(void *arg)
{
    led_t *led = (led_t *)arg;

    TickType_t period =
        pdMS_TO_TICKS(led->interval_ms);

    if (period == 0) {
        abort();
    }

    TickType_t previous =
        xTaskGetTickCount();

    while (1) {
        vTaskDelayUntil(&previous, period);

        led->action(led);
    }
}

void app_main(void)
{
    for (size_t i = 0; i < LED_COUNT; i++) {
        led_init(&leds[i]);

        BaseType_t result = xTaskCreate(
            led_task,
            "led_task",
            LED_TASK_STACK_SIZE,
            &leds[i],
            LED_TASK_PRIORITY,
            NULL
        );

        if (result != pdPASS) {
            abort();
        }
    }
}