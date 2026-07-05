#include "buzzer.h"

#include <stddef.h>

#include "esp_timer.h"

static buzzer_config_t buzzer_config;
static esp_timer_handle_t player_timer;

static const melody_t *active_melody;
static const melody_t *melody_list;
static int melody_list_count;
static int current_melody = -1;
static int current_note;
static uint32_t elapsed_ms;
static bool initialized;
static bool playing;
static bool looping;

static esp_err_t set_tone(note_freq_t freq)
{
    uint32_t duty = buzzer_config.duty;

    if (freq == NOTE_REST) {
        duty = 0;
    } else {
        ledc_set_freq(buzzer_config.speed_mode, buzzer_config.timer_num, (uint32_t)freq);
    }
    ledc_set_duty(buzzer_config.speed_mode, buzzer_config.channel, duty);

    return ledc_update_duty(buzzer_config.speed_mode, buzzer_config.channel);
}


static void player_tick(void *arg)
{
    (void)arg;

    if (!playing || active_melody == NULL || active_melody->notes == NULL || active_melody->len <= 0) {
        return;
    }

    elapsed_ms += buzzer_config.tick_ms;

    if (elapsed_ms < active_melody->notes[current_note].duration_ms) {
        return;
    }

    elapsed_ms = 0;
    current_note++;

    if (current_note >= active_melody->len) {
        if (!looping) {
            buzzer_stop();
            return;
        }

        current_note = 0;
    }

    (void)set_tone(active_melody->notes[current_note].freq);
}

esp_err_t buzzer_init(const buzzer_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    buzzer_config = *config;

    const ledc_timer_config_t timer_config = {
        .speed_mode = buzzer_config.speed_mode,
        .timer_num = buzzer_config.timer_num,
        .duty_resolution = buzzer_config.duty_resolution,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    esp_err_t result = ledc_timer_config(&timer_config);

    if (result != ESP_OK) {
        return result;
    }

    const ledc_channel_config_t channel_config = {
        .gpio_num = buzzer_config.gpio_num,
        .speed_mode = buzzer_config.speed_mode,
        .channel = buzzer_config.channel,
        .timer_sel = buzzer_config.timer_num,
        .duty = 0,
        .hpoint = 0,
    };

    result = ledc_channel_config(&channel_config);

    if (result != ESP_OK) {
        return result;
    }

    const esp_timer_create_args_t player_timer_args = {
        .callback = player_tick,
        .name = "buzzer_player",
    };

    result = esp_timer_create(&player_timer_args, &player_timer);

    if (result != ESP_OK) {
        return result;
    }

    initialized = true;
    return ESP_OK;
}

esp_err_t buzzer_stop(void)
{
    if (!initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    (void)esp_timer_stop(player_timer);
    playing = false;
    active_melody = NULL;
    current_note = 0;
    elapsed_ms = 0;

    return set_tone(NOTE_REST);
}


esp_err_t buzzer_play_melody(const melody_t *melody, bool loop)
{
    if (!initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (melody == NULL || melody->notes == NULL || melody->len <= 0) {
        return ESP_ERR_INVALID_ARG;
    }
    (void)esp_timer_stop(player_timer);

    active_melody = melody;
    current_note = 0;
    elapsed_ms = 0;
    playing = true;
    looping = loop;

    set_tone(active_melody->notes[current_note].freq);
    return esp_timer_start_periodic(player_timer, buzzer_config.tick_ms * 1000ULL);
}

esp_err_t buzzer_play_melody_at(const melody_t *melodies, int melody_count, int melody_index, bool loop)
{
    if (melodies == NULL || melody_count <= 0 || melody_index < 0 || melody_index >= melody_count) {
        return ESP_ERR_INVALID_ARG;
    }

    melody_list = melodies;
    melody_list_count = melody_count;
    current_melody = melody_index;

    return buzzer_play_melody(&melodies[melody_index], loop);
}

esp_err_t buzzer_next_melody(const melody_t *melodies, int melody_count, bool loop)
{
    if (melodies == NULL || melody_count <= 0) {
        return ESP_ERR_INVALID_ARG;
    }

    if (melodies != melody_list || melody_count != melody_list_count || current_melody < 0) {
        current_melody = 0;
    } else {
        current_melody = (current_melody + 1) % melody_count;
    }

    melody_list = melodies;
    melody_list_count = melody_count;

    return buzzer_play_melody(&melodies[current_melody], loop);
}

esp_err_t buzzer_play_tick(const melody_t *tick_sound)
{
    return buzzer_play_melody(tick_sound, false);
}

bool buzzer_is_playing(void)
{
    return playing;
}

int buzzer_current_melody_index(void)
{
    return current_melody;
}
