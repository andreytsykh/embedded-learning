#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "safe.h"
#include "encoder.h"
#include "seven_segment.h"
#include "safe_callbacks.h"
#include "safe_indication.h"
#include "button.h"
#include "buzzer.h"

#define TAG "ENCODER_DISPLAY"

/*
 * GPIO 7-сегментного індикатора.
 */
#define SEG_A_GPIO GPIO_NUM_4
#define SEG_B_GPIO GPIO_NUM_5
#define SEG_C_GPIO GPIO_NUM_6
#define SEG_D_GPIO GPIO_NUM_7
#define SEG_E_GPIO GPIO_NUM_15
#define SEG_F_GPIO GPIO_NUM_16
#define SEG_G_GPIO GPIO_NUM_17

/* LED */
#define LED_1_GPIO GPIO_NUM_11
#define LED_2_GPIO GPIO_NUM_36
#define LED_3_GPIO GPIO_NUM_37
#define LED_4_GPIO GPIO_NUM_12

#define LED_LOCK_GPIO GPIO_NUM_8
#define LED_UNLOCK_GPIO GPIO_NUM_41

#define BUZZER_GPIO GPIO_NUM_9

/*
 * GPIO енкодера.
 */
#define ENCODER_A_GPIO GPIO_NUM_18
#define ENCODER_B_GPIO GPIO_NUM_21
#define ENCODER_SW_GPIO GPIO_NUM_10

#define BUTTON_DEBOUNCE_US      50000ULL
#define BUTTON_LONG_PRESS_US    4000000ULL

#define ENCODER_COUNTS_PER_DETENT 4
#define ENCODER_GLITCH_FILTER_NS  1000U

#define MAIN_LOOP_DELAY_MS 10U

static void process_encoder_event(
    encoder_event_t event,
    uint8_t *counter)
{
    switch (event)
    {
    case ENCODER_EVENT_CW:
        safe_process_event(SAFE_EVENT_ROTATE_CW);
        break;
    case ENCODER_EVENT_CCW:
        safe_process_event(SAFE_EVENT_ROTATE_CCW);
        break;
    default:
        break;
    }
}

static void on_button_short_press(void *ctx) {
    ESP_LOGI("on_button_short_press", "SHORT");
    safe_process_event(SAFE_EVENT_RESET);
}

static void on_button_long_press(void *ctx) {
   ESP_LOGI("on_button_long_press", "LONG");
}

static button_t button = {
        .gpio           = ENCODER_SW_GPIO,
        .state          = BUTTON_IDLE,
        .debounce_us    = BUTTON_DEBOUNCE_US,
        .long_press_us  = BUTTON_LONG_PRESS_US,
        .on_short_press = on_button_short_press,
        .on_long_press  = on_button_long_press,
        .ctx            = NULL,
    };

void app_main(void)
{
    gpio_install_isr_service(0);
    button_init(&button);

    const safe_indication_config_t led_config = {
        .gpio_bit_0 = LED_1_GPIO,
        .gpio_bit_1 = LED_2_GPIO,
        .gpio_bit_2 = LED_3_GPIO,
        .gpio_bit_3 = LED_4_GPIO,
        .gpio_lock = LED_LOCK_GPIO,
        .gpio_unlock = LED_UNLOCK_GPIO,
        .active_high = true
    };

    safe_indication_init(&led_config);
    safe_indication_on();

    const seven_segment_config_t display_config = {
        .segment_a_gpio = SEG_A_GPIO,
        .segment_b_gpio = SEG_B_GPIO,
        .segment_c_gpio = SEG_C_GPIO,
        .segment_d_gpio = SEG_D_GPIO,
        .segment_e_gpio = SEG_E_GPIO,
        .segment_f_gpio = SEG_F_GPIO,
        .segment_g_gpio = SEG_G_GPIO
    };

    const encoder_config_t encoder_config = {
        .gpio_a = ENCODER_A_GPIO,
        .gpio_b = ENCODER_B_GPIO,
        .counts_per_detent = ENCODER_COUNTS_PER_DETENT,
        .glitch_filter_ns = ENCODER_GLITCH_FILTER_NS
    };

    const safe_config_t safe_config = {
        .on_digit_changed = safe_digit_changed_cb
    };

    const buzzer_config_t buzzer_config = BUZZER_CONFIG_DEFAULT(BUZZER_GPIO);

    ESP_ERROR_CHECK(
        seven_segment_init(&display_config)
    );

    ESP_ERROR_CHECK(
        encoder_init(&encoder_config)
    );

    ESP_ERROR_CHECK(
        buzzer_init(&buzzer_config)
    );

    uint8_t counter = 0U;

    ESP_ERROR_CHECK(
        seven_segment_show_digit(counter)
    );

    ESP_LOGI(
        TAG,
        "Initialized, counter: %u",
        (unsigned int)counter
    );

    safe_init(&safe_config);

    while (true) {
        encoder_event_t event;

        /*
         * encoder_get_event() повертає максимум одну подію.
         * do-while забирає всі накопичені події.
         */
        do {
            event = encoder_get_event();

            process_encoder_event(event, &counter);

        } while (event != ENCODER_EVENT_NONE);

        vTaskDelay(
            pdMS_TO_TICKS(MAIN_LOOP_DELAY_MS)
        );
    }
}
