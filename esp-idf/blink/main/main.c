#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#define BUTTON_GPIO GPIO_NUM_4

static const char* TAG = "Button Interrupt";

static volatile uint32_t irq_count = 0;

static void IRAM_ATTR button_isr_handler(void *arg)
{
    irq_count++;
}

void app_main(void)
{
    gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // Кнопка має власний pull down резистор
        .intr_type = GPIO_INTR_POSEDGE // Так як кнопка в стану спокою має 0 
    };

    gpio_config(&button_config);
    gpio_install_isr_service(0);

    gpio_isr_handler_add(
        BUTTON_GPIO,
        button_isr_handler,
        NULL
    );

    ESP_LOGI(TAG, "Started. GPIO=%d, interrupt=NEGEDGE", BUTTON_GPIO);
    ESP_LOGI(TAG, "Initial level: %d", gpio_get_level(BUTTON_GPIO));

    uint32_t last_count = 0;

    while (1) {
        if (last_count != irq_count) {
            last_count = irq_count;

            ESP_LOGI(TAG, "Interrupt count: %lu", last_count);
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}