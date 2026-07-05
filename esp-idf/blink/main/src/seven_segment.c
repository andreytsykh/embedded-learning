#include "seven_segment.h"

#include <stdbool.h>
#include <stddef.h>

#define SEVEN_SEGMENT_COUNT 7U

#define SEGMENT_ON  0U
#define SEGMENT_OFF 1U

#define SEG_A (1U << 0U)
#define SEG_B (1U << 1U)
#define SEG_C (1U << 2U)
#define SEG_D (1U << 3U)
#define SEG_E (1U << 4U)
#define SEG_F (1U << 5U)
#define SEG_G (1U << 6U)

static gpio_num_t s_segment_gpios[SEVEN_SEGMENT_COUNT];

static bool s_is_initialized = false;

/*
 * Маски сегментів для цифр 0-9.
 *
 * bit 0 -> A
 * bit 1 -> B
 * bit 2 -> C
 * bit 3 -> D
 * bit 4 -> E
 * bit 5 -> F
 * bit 6 -> G
 */
static const uint8_t s_digit_segments[10] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,         /* 0 */
    SEG_B | SEG_C,                                         /* 1 */
    SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,                 /* 2 */
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,                 /* 3 */
    SEG_B | SEG_C | SEG_F | SEG_G,                         /* 4 */
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,                 /* 5 */
    SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,         /* 6 */
    SEG_A | SEG_B | SEG_C,                                 /* 7 */
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G, /* 8 */
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G          /* 9 */
};

static esp_err_t seven_segment_set_mask(uint8_t segment_mask)
{
    if (!s_is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t index = 0U; index < SEVEN_SEGMENT_COUNT; ++index) {
        const bool segment_enabled =
            ((segment_mask >> index) & 1U) != 0U;

        const uint32_t gpio_level =
            segment_enabled ? SEGMENT_ON : SEGMENT_OFF;

        const esp_err_t result =
            gpio_set_level(s_segment_gpios[index], gpio_level);

        if (result != ESP_OK) {
            return result;
        }
    }

    return ESP_OK;
}

esp_err_t seven_segment_init(const seven_segment_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    s_segment_gpios[0] = config->segment_a_gpio;
    s_segment_gpios[1] = config->segment_b_gpio;
    s_segment_gpios[2] = config->segment_c_gpio;
    s_segment_gpios[3] = config->segment_d_gpio;
    s_segment_gpios[4] = config->segment_e_gpio;
    s_segment_gpios[5] = config->segment_f_gpio;
    s_segment_gpios[6] = config->segment_g_gpio;

    uint64_t pin_mask = 0ULL;

    for (uint8_t index = 0U; index < SEVEN_SEGMENT_COUNT; ++index) {
        pin_mask |= 1ULL << (uint32_t)s_segment_gpios[index];
    }

    const gpio_config_t gpio_cfg = {
        .pin_bit_mask = pin_mask,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    const esp_err_t result = gpio_config(&gpio_cfg);

    if (result != ESP_OK) {
        return result;
    }

    s_is_initialized = true;

    return seven_segment_clear();
}

esp_err_t seven_segment_show_digit(uint8_t digit)
{
    if (!s_is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (digit > 9U) {
        return ESP_ERR_INVALID_ARG;
    }

    return seven_segment_set_mask(s_digit_segments[digit]);
}

esp_err_t seven_segment_clear(void)
{
    if (!s_is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    return seven_segment_set_mask(0U);
}