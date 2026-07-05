#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"

typedef enum {
    NOTE_REST = 0,
    NOTE_C  = 262, NOTE_D  = 294, NOTE_E  = 330, NOTE_F  = 349,
    NOTE_G  = 392, NOTE_A  = 440, NOTE_B  = 494, NOTE_C5 = 523,
    NOTE_D5 = 587, NOTE_E5 = 659, NOTE_F5 = 698, NOTE_G5 = 784,
    NOTE_A5 = 880,
} note_freq_t;

typedef struct {
    note_freq_t freq;
    uint16_t duration_ms;
} note_t;

typedef struct {
    const note_t *notes;
    int len;
    const char *name;
} melody_t;

typedef struct {
    gpio_num_t gpio_num;
    ledc_mode_t speed_mode;
    ledc_timer_t timer_num;
    ledc_channel_t channel;
    ledc_timer_bit_t duty_resolution;
    uint32_t duty;
    uint32_t tick_ms;
} buzzer_config_t;

#define BUZZER_CONFIG_DEFAULT(gpio)          \
    {                                        \
        .gpio_num = (gpio),                  \
        .speed_mode = LEDC_LOW_SPEED_MODE,   \
        .timer_num = LEDC_TIMER_0,           \
        .channel = LEDC_CHANNEL_0,           \
        .duty_resolution = LEDC_TIMER_10_BIT,\
        .duty = 512,                         \
        .tick_ms = 10,                       \
    }

esp_err_t buzzer_init(const buzzer_config_t *config);

esp_err_t buzzer_stop(void);

esp_err_t buzzer_play_melody(const melody_t *melody, bool loop);
esp_err_t buzzer_play_melody_at(const melody_t *melodies, int melody_count, int melody_index, bool loop);
esp_err_t buzzer_next_melody(const melody_t *melodies, int melody_count, bool loop);
esp_err_t buzzer_play_tick(const melody_t *tick_sound);

bool buzzer_is_playing(void);
int buzzer_current_melody_index(void);
