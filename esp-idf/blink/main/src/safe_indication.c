#include "safe_indication.h"

#include <stddef.h>

#include "esp_timer.h"

#define SAFE_INDICATION_LED_COUNT        6U
#define SAFE_INDICATION_BLINK_PERIOD_US  500000ULL

static safe_indication_config_t indication_config;

static uint8_t led_on_mask = 0U;
static uint8_t led_blink_mask = 0U;

static bool blink_phase = false;
static bool initialized = false;
static bool enabled = false;

static esp_timer_handle_t blink_timer = NULL;


static esp_err_t leds_apply(
    const safe_indication_config_t *cfg)
{
    uint8_t output_state = 0U;

    if (enabled) {
        output_state = led_on_mask;

        if (blink_phase) {
            output_state |= led_blink_mask;
        }
    }

    esp_err_t result = gpio_set_level(
        cfg->gpio_bit_0,
        (uint32_t)((output_state >> 0U) & 1U)
    );

    result = gpio_set_level(
        cfg->gpio_bit_1,
        (uint32_t)((output_state >> 1U) & 1U)
    );

    result = gpio_set_level(
        cfg->gpio_bit_2,
        (uint32_t)((output_state >> 2U) & 1U)
    );

    result = gpio_set_level(
        cfg->gpio_bit_3,
        (uint32_t)((output_state >> 3U) & 1U)
    );

    result = gpio_set_level(
        cfg->gpio_lock,
        (uint32_t)((output_state >> 4U) & 1U)
    );

    return gpio_set_level(
        cfg->gpio_unlock,
        (uint32_t)((output_state >> 5U) & 1U)
    );
}


static void blink_timer_callback(void *arg)
{
    (void)arg;

    blink_phase = !blink_phase;

    (void)leds_apply(&indication_config);
}


esp_err_t safe_indication_init(
    const safe_indication_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    indication_config = *config;

    const uint64_t gpio_mask =
        (1ULL << indication_config.gpio_bit_0) |
        (1ULL << indication_config.gpio_bit_1) |
        (1ULL << indication_config.gpio_bit_2) |
        (1ULL << indication_config.gpio_bit_3) |
        (1ULL << indication_config.gpio_lock) |
        (1ULL << indication_config.gpio_unlock);

    const gpio_config_t gpio_cfg = {
        .pin_bit_mask = gpio_mask,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    esp_err_t result = gpio_config(&gpio_cfg);

    if (result != ESP_OK) {
        return result;
    }

    const esp_timer_create_args_t timer_args = {
        .callback = blink_timer_callback,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "safe_led_blink",
        .skip_unhandled_events = true
    };

    result = esp_timer_create(
        &timer_args,
        &blink_timer
    );

    result = esp_timer_start_periodic(
        blink_timer,
        SAFE_INDICATION_BLINK_PERIOD_US
    );

    if (result != ESP_OK) {
        (void)esp_timer_delete(blink_timer);
        blink_timer = NULL;

        return result;
    }

    led_on_mask = 0U;
    led_blink_mask = 0U;
    blink_phase = false;

    initialized = true;
    enabled = true;

    return leds_apply(&indication_config);
}


esp_err_t safe_indication_show_digit(uint8_t digit)
{
    if (!initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (digit > 9U) {
        return ESP_ERR_INVALID_ARG;
    }

    enabled = true;

    return seven_segment_show_digit(digit);
}


esp_err_t safe_indication_set_led_mode(
    uint8_t index,
    safe_indication_led_mode_t mode)
{
    if (!initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (index >= SAFE_INDICATION_LED_COUNT) {
        return ESP_ERR_INVALID_ARG;
    }

    const uint8_t mask = (uint8_t)(1U << index);

    switch (mode) {
        case SAFE_INDICATION_LED_OFF:
            led_on_mask &= (uint8_t)~mask;
            led_blink_mask &= (uint8_t)~mask;
            break;

        case SAFE_INDICATION_LED_ON:
            led_on_mask |= mask;
            led_blink_mask &= (uint8_t)~mask;
            break;

        case SAFE_INDICATION_LED_BLINK:
            led_on_mask &= (uint8_t)~mask;
            led_blink_mask |= mask;
            break;

        default:
            return ESP_ERR_INVALID_ARG;
    }

    return leds_apply(&indication_config);
}


esp_err_t safe_indication_activate_led(uint8_t index)
{
    return safe_indication_set_led_mode(
        index,
        SAFE_INDICATION_LED_ON
    );
}


esp_err_t safe_indication_blink_led(uint8_t index)
{
    return safe_indication_set_led_mode(
        index,
        SAFE_INDICATION_LED_BLINK
    );
}


esp_err_t safe_indication_deactivate_led(uint8_t index)
{
    return safe_indication_set_led_mode(
        index,
        SAFE_INDICATION_LED_OFF
    );
}


esp_err_t safe_indication_off(void)
{
    if (!initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    led_blink_mask = 0U;
    led_on_mask = 0U;
    enabled = false;

    return leds_apply(&indication_config);
}


esp_err_t safe_indication_on(void)
{
    if (!initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    enabled = true;

    return leds_apply(&indication_config);
}


bool safe_indication_is_enabled(void)
{
    return enabled;
}