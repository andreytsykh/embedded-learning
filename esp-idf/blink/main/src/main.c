#include "freertos/FreeRTOS.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "button.h"
#include "melodies.h"
#include "main.h"

static const char* TAG = "Buzzer";

static int  current_melody = 0;
static int  current_note   = 0;
static int  elapsed_ms     = 0;
static bool playing        = false;

static esp_timer_handle_t player_timer;

static void tone_set(note_freq_t freq) {
    if (freq == NOTE_REST) {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    } else {
        ledc_set_freq(LEDC_MODE, LEDC_TIMER, freq);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 512);
    }
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

static void player_stop(void) {
    playing = false;
    esp_timer_stop(player_timer);
    tone_set(NOTE_REST);
    ESP_LOGI(TAG, "Stopped");
}

static void player_start(int melody_idx) {
    current_melody = melody_idx;
    current_note   = 0;
    elapsed_ms     = 0;
    playing        = true;

    const melody_t *m = &melodies[current_melody];
    ESP_LOGI(TAG, "Playing: %s", m->name);

    tone_set(m->notes[0].freq);
    esp_timer_start_periodic(player_timer, TICK_MS * 1000);
}

static void player_tick(void *arg) {
    if (!playing) return;

    const melody_t *m = &melodies[current_melody];

    elapsed_ms += TICK_MS;

    if (elapsed_ms >= m->notes[current_note].duration_ms) {
        elapsed_ms = 0;
        current_note++;

        if (current_note >= m->len) {
            current_note = 0;
            ESP_LOGI(TAG, "Loop: %s", m->name);
        }

        tone_set(m->notes[current_note].freq);
    }
}

static void on_button_short_press(void *ctx) {
    if (!playing) return;

    esp_timer_stop(player_timer);
    int next = (current_melody + 1) % melodies_count;
    player_start(next);
}

static void on_button_long_press(void *ctx) {
    if (playing) {
        player_stop();
    } else {
        player_start(current_melody);
    }
}

static button_t button = {
    .gpio           = BUTTON_GPIO,
    .state          = BUTTON_IDLE,
    .debounce_us    = BUTTON_DEBOUNCE_US,
    .long_press_us  = BUTTON_LONG_PRESS_US,
    .on_short_press = on_button_short_press,
    .on_long_press  = on_button_long_press,
    .ctx            = NULL,
};

void app_main(void) {
    gpio_install_isr_service(0);
    button_init(&button);

    ledc_timer_config_t timer_cfg = {
        .speed_mode      = LEDC_MODE,
        .timer_num       = LEDC_TIMER,
        .duty_resolution = LEDC_RES,
        .freq_hz         = 1000,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_cfg);

    ledc_channel_config_t ch_cfg = {
        .gpio_num   = BUZZER_GPIO,
        .speed_mode = LEDC_MODE,
        .channel    = LEDC_CHANNEL,
        .timer_sel  = LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0,
    };
    ledc_channel_config(&ch_cfg);

    esp_timer_create_args_t timer_args = {
        .callback = player_tick,
        .name     = "player"
    };
    esp_timer_create(&timer_args, &player_timer);

    ESP_LOGI(TAG, "Ready. Long press to start, short press to change melody");
}