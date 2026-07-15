#include "main.h"

#include <stdbool.h>
#include <stddef.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "button.h"
#include "buzzer.h"
#include "display.h"
#include "encoder.h"
#include "safe.h"
#include "safe_callbacks.h"
#include "safe_indication.h"
#include "stm_link.h"

static void on_button_short_press(void *ctx)
{
    (void)ctx;
    ESP_LOGI(MAIN_TAG, "Reset button pressed");
    safe_process_event(SAFE_EVENT_RESET);
}

static void on_button_long_press(void *ctx)
{
    (void)ctx;
    ESP_LOGI(MAIN_TAG, "Reset button long press");
}

static button_t button = {
    .gpio = ENCODER_SW_GPIO,
    .state = BUTTON_IDLE,
    .debounce_us = BUTTON_DEBOUNCE_US,
    .long_press_us = BUTTON_LONG_PRESS_US,
    .on_short_press = on_button_short_press,
    .on_long_press = on_button_long_press,
    .ctx = NULL,
};

static void initialize_button(void)
{
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    button_init(&button);
}

static void initialize_safe_indication(void)
{
    const safe_indication_config_t config = {
        .gpio_bit_0 = LED_1_GPIO,
        .gpio_bit_1 = LED_2_GPIO,
        .gpio_bit_2 = LED_3_GPIO,
        .gpio_bit_3 = LED_4_GPIO,
        .gpio_lock = LED_LOCK_GPIO,
        .gpio_unlock = LED_UNLOCK_GPIO,
        .active_high = true,
    };

    ESP_ERROR_CHECK(safe_indication_init(&config));
    ESP_ERROR_CHECK(safe_indication_on());
}

static void initialize_encoder(void)
{
    const encoder_config_t config = {
        .gpio_a = ENCODER_A_GPIO,
        .gpio_b = ENCODER_B_GPIO,
        .counts_per_detent = ENCODER_COUNTS_PER_DETENT,
        .glitch_filter_ns = ENCODER_GLITCH_FILTER_NS,
    };

    ESP_ERROR_CHECK(encoder_init(&config));
}

static void initialize_buzzer(void)
{
    const buzzer_config_t config = BUZZER_CONFIG_DEFAULT(BUZZER_GPIO);
    ESP_ERROR_CHECK(buzzer_init(&config));
}

static void initialize_safe(void)
{
    const safe_config_t config = {
        .on_digit_changed = safe_digit_changed_cb,
    };

    safe_init(&config);
}

static void initialize_stm_link(void)
{
    ESP_ERROR_CHECK(stm_link_init());
    ESP_ERROR_CHECK(stm_send_command(STM_COMMAND_CLOSE));
}

static void initialize_application(void)
{
    initialize_button();
    ESP_ERROR_CHECK(oled_display_init());
    initialize_safe_indication();
    initialize_encoder();
    initialize_buzzer();
    initialize_stm_link();
    initialize_safe();

    ESP_LOGI(MAIN_TAG, "Application initialized");
}

static void process_encoder_event(encoder_event_t event)
{
    switch (event) {
        case ENCODER_EVENT_CW:
            safe_process_event(SAFE_EVENT_ROTATE_CW);
            break;

        case ENCODER_EVENT_CCW:
            safe_process_event(SAFE_EVENT_ROTATE_CCW);
            break;

        case ENCODER_EVENT_NONE:
        default:
            break;
    }
}

static void process_pending_encoder_events(void)
{
    encoder_event_t event;

    do {
        event = encoder_get_event();
        process_encoder_event(event);
    } while (event != ENCODER_EVENT_NONE);
}

void app_main(void)
{
    initialize_application();

    while (true) {
        process_pending_encoder_events();
        vTaskDelay(pdMS_TO_TICKS(MAIN_LOOP_DELAY_MS));
    }
}
