#include <stdbool.h>
#include <stdint.h>

#include "traffic_light.h"
#include "config.h"

#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_check.h"

typedef enum
{
    TRAFFIC_MODE_STOPPED,
    TRAFFIC_MODE_NORMAL,
    TRAFFIC_MODE_YELLOW_BLINK
} traffic_light_mode_t;


typedef struct
{
    gpio_num_t green_gpio;
    gpio_num_t yellow_gpio;
    gpio_num_t red_gpio;

    traffic_light_state_t state;

    esp_timer_handle_t state_timer;
    esp_timer_handle_t blink_timer;

    bool green_on;
    bool yellow_on;
    bool red_on;

    traffic_light_mode_t mode;
} traffic_light_t;

static traffic_light_t tl = {
    .green_gpio = TRAFFIC_LIGHT_GREEN_GPIO,
    .yellow_gpio = TRAFFIC_LIGHT_YELLOW_GPIO,
    .red_gpio = TRAFFIC_LIGHT_RED_GPIO,
    .state = RED,
    .mode = TRAFFIC_MODE_STOPPED};

void traffic_light_set_mode(traffic_light_mode_t mode);
static void state_timer_cb(void *arg);
static void blink_timer_cb(void *arg);

static void set_lights(bool red, bool yellow, bool green)
{
    tl.red_on = red;
    tl.yellow_on = yellow;
    tl.green_on = green;

    gpio_set_level(tl.red_gpio, tl.red_on);
    gpio_set_level(tl.yellow_gpio, tl.yellow_on);
    gpio_set_level(tl.green_gpio, tl.green_on);
}

static void stop_timer_if_running(esp_timer_handle_t timer)
{
    esp_err_t err = esp_timer_stop(timer);

    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        ESP_ERROR_CHECK(err);
    }
}

static uint64_t get_state_duration_us(void)
{
    switch (tl.state)
    {
    case RED:
        return TRAFFIC_LIGHT_RED_US;

    case RED_YELLOW:
        return TRAFFIC_LIGHT_RED_YELLOW_US;

    case GREEN:
        return TRAFFIC_LIGHT_GREEN_US;

    case GREEN_BLINKING:
        return TRAFFIC_LIGHT_GREEN_BLINK_US;

    case YELLOW:
        return TRAFFIC_LIGHT_YELLOW_US;

    case YELLOW_BLINKING:
        return TRAFFIC_LIGHT_YELLOW_US;

    default:
        return TRAFFIC_LIGHT_BLINK_US;
    }
}

static void start_state_timer(void)
{
    ESP_ERROR_CHECK(
        esp_timer_start_once(
            tl.state_timer,
            get_state_duration_us()));
}

static void enter_state(traffic_light_state_t next_state)
{
    stop_timer_if_running(tl.state_timer);
    stop_timer_if_running(tl.blink_timer);

    tl.state = next_state;

    switch (tl.state)
    {
    case RED:
        set_lights(true, false, false);
        break;

    case RED_YELLOW:
        set_lights(true, true, false);
        break;

    case GREEN:
        set_lights(false, false, true);
        break;

    case GREEN_BLINKING:
        set_lights(false, false, true);

        ESP_ERROR_CHECK(
            esp_timer_start_periodic(
                tl.blink_timer,
                TRAFFIC_LIGHT_BLINK_US));
        break;

    case YELLOW:
        set_lights(false, true, false);
        break;

    case YELLOW_BLINKING:
        set_lights(false, true, false);
        ESP_ERROR_CHECK(
            esp_timer_start_periodic(
                tl.blink_timer,
                TRAFFIC_LIGHT_BLINK_US));
        return;

    default:
        break;
    }

    start_state_timer();
}

static traffic_light_state_t get_next_state(void)
{
    switch (tl.state)
    {
    case RED:
        return RED_YELLOW;

    case RED_YELLOW:
        return GREEN;

    case GREEN:
        return GREEN_BLINKING;

    case GREEN_BLINKING:
        return YELLOW;

    case YELLOW:
        return RED;

    case YELLOW_BLINKING:
        return RED;

    default:
        return RED;
    }
}

static void state_timer_cb(void *arg)
{
    enter_state(get_next_state());
}

static void blink_timer_cb(void *arg)
{
    if (tl.state == GREEN_BLINKING)
    {
        set_lights(false, false, !tl.green_on);
    }
    else if (tl.state == YELLOW_BLINKING)
    {
        set_lights(false, !tl.yellow_on, false);
    }
}

void traffic_light_init(void)
{
    gpio_config_t leds_config = {
        .pin_bit_mask =
            (1ULL << tl.green_gpio) |
            (1ULL << tl.yellow_gpio) |
            (1ULL << tl.red_gpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};

    ESP_ERROR_CHECK(gpio_config(&leds_config));

    esp_timer_create_args_t state_args = {
        .callback = state_timer_cb,
        .name = "traffic_state"};

    ESP_ERROR_CHECK(
        esp_timer_create(&state_args, &tl.state_timer));

    esp_timer_create_args_t blink_args = {
        .callback = blink_timer_cb,
        .name = "traffic_blink"};

    ESP_ERROR_CHECK(
        esp_timer_create(&blink_args, &tl.blink_timer));

    set_lights(false, false, false);
    tl.state = RED;
    tl.mode = TRAFFIC_MODE_STOPPED;
}

void traffic_light_start(void)
{
    if (tl.mode != TRAFFIC_MODE_STOPPED)
    {
        return;
    }
    tl.mode = TRAFFIC_MODE_NORMAL;
    enter_state(tl.state);
}

void traffic_light_toggle_yellow_blink(void)
{
    if (tl.mode == TRAFFIC_MODE_YELLOW_BLINK)
    {
        tl.mode = TRAFFIC_MODE_NORMAL;
        enter_state(RED);
    }
    else
    {
        tl.mode = TRAFFIC_MODE_YELLOW_BLINK;
        enter_state(YELLOW_BLINKING);
    }
}

traffic_light_state_t traffic_light_get_state(void)
{
    return tl.state;
}