#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BUTTON_GPIO GPIO_NUM_4
#define POLL_MS     10
#define STABLE_COUNT 5  // скільки однакових читань підряд = стабільний стан

static const char* TAG = "Button Interrupt";

typedef enum {
    BTN_IDLE,
    BTN_PRESSING,
    BTN_PRESSED,
    BTN_RELEASING,
} btn_state_t;

void app_main(void)
{
    gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&button_config);

    ESP_LOGI(TAG, "Started. GPIO=%d, polling=%dms", BUTTON_GPIO, POLL_MS);

    btn_state_t state = BTN_IDLE;
    uint8_t stable = 0;
    uint32_t press_count = 0;

    while (1) {
        int level = gpio_get_level(BUTTON_GPIO);

        switch (state) {
            case BTN_IDLE:
                if (level == 1) {
                    state = BTN_PRESSING;
                    stable = 0;
                }
                break;

            case BTN_PRESSING:
                if (level == 1) {
                    stable++;
                    if (stable >= STABLE_COUNT) {
                        state = BTN_PRESSED;
                        press_count++;
                        ESP_LOGI(TAG, "VALID PRESS #%lu", press_count);
                    }
                } else {
                    state = BTN_IDLE;
                    stable = 0;
                }
                break;

            case BTN_PRESSED:
                if (level == 0) {
                    state = BTN_RELEASING;
                    stable = 0;
                }
                break;

            case BTN_RELEASING:
                if (level == 0) {
                    stable++;
                    if (stable >= STABLE_COUNT) {
                        state = BTN_IDLE;
                        ESP_LOGI(TAG, "Button released");
                    }
                } else {
                    state = BTN_PRESSED;
                    stable = 0;
                }
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(POLL_MS));
    }
}