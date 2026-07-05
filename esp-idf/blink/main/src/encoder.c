#include "encoder.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "driver/pulse_cnt.h"

#define ENCODER_PCNT_HIGH_LIMIT  30000
#define ENCODER_PCNT_LOW_LIMIT  (-30000)

static pcnt_unit_handle_t s_pcnt_unit = NULL;
static pcnt_channel_handle_t s_channel_a = NULL;
static pcnt_channel_handle_t s_channel_b = NULL;

static int32_t s_counts_per_detent = 1;
static int32_t s_accumulated_count = 0;

static bool s_is_initialized = false;

esp_err_t encoder_init(const encoder_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (s_is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (config->gpio_a == config->gpio_b) {
        return ESP_ERR_INVALID_ARG;
    }

    if (config->counts_per_detent <= 0) {
        return ESP_ERR_INVALID_ARG;
    }

    const pcnt_unit_config_t unit_config = {
        .high_limit = ENCODER_PCNT_HIGH_LIMIT,
        .low_limit = ENCODER_PCNT_LOW_LIMIT
    };

    esp_err_t result = pcnt_new_unit(
        &unit_config,
        &s_pcnt_unit
    );

    if (result != ESP_OK) {
        return result;
    }

    const pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = config->gpio_a,
        .level_gpio_num = config->gpio_b
    };

    result = pcnt_new_channel(
        s_pcnt_unit,
        &chan_a_config,
        &s_channel_a
    );

    if (result != ESP_OK) {
        return result;
    }

    const pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = config->gpio_b,
        .level_gpio_num = config->gpio_a
    };

    result = pcnt_new_channel(
        s_pcnt_unit,
        &chan_b_config,
        &s_channel_b
    );

    if (result != ESP_OK) {
        return result;
    }

    result = pcnt_channel_set_edge_action(
        s_channel_a,
        PCNT_CHANNEL_EDGE_ACTION_DECREASE,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE
    );

    if (result != ESP_OK) {
        return result;
    }

    result = pcnt_channel_set_level_action(
        s_channel_a,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP,
        PCNT_CHANNEL_LEVEL_ACTION_INVERSE
    );

    if (result != ESP_OK) {
        return result;
    }

    result = pcnt_channel_set_edge_action(
        s_channel_b,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE,
        PCNT_CHANNEL_EDGE_ACTION_DECREASE
    );

    if (result != ESP_OK) {
        return result;
    }

    result = pcnt_channel_set_level_action(
        s_channel_b,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP,
        PCNT_CHANNEL_LEVEL_ACTION_INVERSE
    );

    if (result != ESP_OK) {
        return result;
    }

    if (config->glitch_filter_ns > 0U) {
        const pcnt_glitch_filter_config_t filter_config = {
            .max_glitch_ns = config->glitch_filter_ns
        };

        result = pcnt_unit_set_glitch_filter(
            s_pcnt_unit,
            &filter_config
        );

        if (result != ESP_OK) {
            return result;
        }
    }

    result = pcnt_unit_enable(s_pcnt_unit);

    if (result != ESP_OK) {
        return result;
    }

    result = pcnt_unit_clear_count(s_pcnt_unit);

    if (result != ESP_OK) {
        return result;
    }

    result = pcnt_unit_start(s_pcnt_unit);

    if (result != ESP_OK) {
        return result;
    }

    s_counts_per_detent = config->counts_per_detent;
    s_accumulated_count = 0;
    s_is_initialized = true;

    return ESP_OK;
}

encoder_event_t encoder_get_event(void)
{
    if (!s_is_initialized) {
        return ENCODER_EVENT_NONE;
    }

    int count = 0;

    if (pcnt_unit_get_count(s_pcnt_unit, &count) != ESP_OK) {
        return ENCODER_EVENT_NONE;
    }

    if (count != 0) {
        if (pcnt_unit_clear_count(s_pcnt_unit) != ESP_OK) {
            return ENCODER_EVENT_NONE;
        }

        s_accumulated_count += (int32_t)count;
    }

    if (s_accumulated_count >= s_counts_per_detent) {
        s_accumulated_count -= s_counts_per_detent;
        return ENCODER_EVENT_CW;
    }

    if (s_accumulated_count <= -s_counts_per_detent) {
        s_accumulated_count += s_counts_per_detent;
        return ENCODER_EVENT_CCW;
    }

    return ENCODER_EVENT_NONE;
}

esp_err_t encoder_reset(void)
{
    if (!s_is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    const esp_err_t result =
        pcnt_unit_clear_count(s_pcnt_unit);

    if (result != ESP_OK) {
        return result;
    }

    s_accumulated_count = 0;

    return ESP_OK;
}