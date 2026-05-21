#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

#define RELAY_CONTROL_GPIO GPIO_NUM_4
#define RELAY_CONTACT_GPIO GPIO_NUM_18

#define RELAY_ON 0
#define RELAY_OFF 1

#define MEASUREMENTS_COUNT 10

static const char *TAG = "RELAY_TEST";

static volatile bool contact_triggered = false;
static volatile int64_t contact_time_us = 0;

static int64_t measurements_ms[MEASUREMENTS_COUNT];

static void IRAM_ATTR relay_isr_handler(void *arg)
{
    contact_time_us = esp_timer_get_time();
    contact_triggered = true;
}

void app_main(void)
{
    gpio_config_t relay_conf = {
        .pin_bit_mask = 1ULL << RELAY_CONTROL_GPIO,
        .mode = GPIO_MODE_OUTPUT_OD, // OPEN DRAIN
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&relay_conf);

    gpio_config_t contact_conf = {
        .pin_bit_mask = 1ULL << RELAY_CONTACT_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE};
    gpio_config(&contact_conf);

    //
    // ISR
    //
    gpio_install_isr_service(0);
    gpio_isr_handler_add(RELAY_CONTACT_GPIO, relay_isr_handler, NULL);


    gpio_set_level(RELAY_CONTROL_GPIO, RELAY_OFF);
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "Starting measurements...");

    for (int i = 0; i < MEASUREMENTS_COUNT; i++)
    {
        contact_triggered = false;
        contact_time_us = 0;
        vTaskDelay(pdMS_TO_TICKS(1000));

        int64_t start_time_us = esp_timer_get_time();

        gpio_set_level(RELAY_CONTROL_GPIO, RELAY_ON);

        int timeout = 1000;

        while (!contact_triggered && timeout > 0)
        {
            vTaskDelay(pdMS_TO_TICKS(1));
            timeout--;
        }

        if (contact_triggered)
        {
            int64_t reaction_time_us =
                contact_time_us - start_time_us;

            double reaction_time_ms =
                reaction_time_us / 1000.0;

            measurements_ms[i] = reaction_time_ms;

            ESP_LOGI(
                TAG,
                "Measurement %d: %.3f ms",
                i + 1,
                reaction_time_ms);
        }
        else
        {
            measurements_ms[i] = -1;
            ESP_LOGW(
                TAG,
                "Measurement %d: TIMEOUT",
                i + 1);
        }

        gpio_set_level(RELAY_CONTROL_GPIO, RELAY_OFF);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    int64_t sum = 0;
    int valid = 0;

    ESP_LOGI(TAG, "========== RESULTS ==========");

    for (int i = 0; i < MEASUREMENTS_COUNT; i++)
    {

        ESP_LOGI(
            TAG,
            "%d -> %lld ms",
            i + 1,
            measurements_ms[i]);

        if (measurements_ms[i] >= 0)
        {
            sum += measurements_ms[i];
            valid++;
        }
    }
    if (valid > 0)
    {

        double average =
            (double)sum / valid;

        ESP_LOGI(
            TAG,
            "AVERAGE: %.2f ms",
            average);
    }
    ESP_LOGI(TAG, "Finished");
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}