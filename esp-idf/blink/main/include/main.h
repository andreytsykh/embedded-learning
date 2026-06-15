#pragma once

#include "driver/ledc.h"
#include "driver/gpio.h"

// GPIO
#define BUZZER_GPIO             GPIO_NUM_4
#define BUTTON_GPIO             GPIO_NUM_6

// BUTTON
#define BUTTON_DEBOUNCE_US      50000ULL
#define BUTTON_LONG_PRESS_US    2000000ULL

// PLAYER
#define TICK_MS                 50

// LEDC
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_RES                LEDC_TIMER_10_BIT