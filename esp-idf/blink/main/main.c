#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#define RELAY_GPIO       GPIO_NUM_4
#define LED_ON_GPIO      GPIO_NUM_5
#define LED_WAIT_GPIO    GPIO_NUM_6

#define PERIOD_US        (3600ULL * 1000 * 1000) // 10с =  10ULL  * 1000 * 1000
#define RUN_DURATION_US  (900ULL  * 1000 * 1000) // 2.5с = 2500ULL * 1000

static const char* TAG = "Fan";

static esp_timer_handle_t period_timer;
static esp_timer_handle_t stop_timer;
static bool fan_running = false;

static void set_leds(bool running) {
    gpio_set_level(LED_ON_GPIO,   running ? 1 : 0);
    gpio_set_level(LED_WAIT_GPIO, running ? 0 : 1);
}

static void fan_stop_cb(void *arg) {
    if (!fan_running) return;

    gpio_set_level(RELAY_GPIO, 1);
    fan_running = false;
    set_leds(false);
    ESP_LOGI(TAG, "Fan OFF — waiting for next cycle");
}

static void fan_start_cb(void *arg) {
    if (fan_running) {
        ESP_LOGW(TAG, "Fan already running, skipping");
        return;
    }

    gpio_set_level(RELAY_GPIO, 0);
    fan_running = true;
    set_leds(true);
    ESP_LOGI(TAG, "Fan ON");

    esp_timer_start_once(stop_timer, RUN_DURATION_US);
}

void app_main(void) {
    gpio_config_t relay_cfg = {
        .pin_bit_mask = (1ULL << RELAY_GPIO),
        .mode = GPIO_MODE_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&relay_cfg);
    gpio_set_level(RELAY_GPIO, 1);

    gpio_config_t led_cfg = {
        .pin_bit_mask = (1ULL << LED_ON_GPIO) | (1ULL << LED_WAIT_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&led_cfg);

    set_leds(false);

    esp_timer_create_args_t stop_args = {
        .callback = fan_stop_cb,
        .name = "fan_stop"
    };
    esp_timer_create(&stop_args, &stop_timer);

    esp_timer_create_args_t period_args = {
        .callback = fan_start_cb,
        .name = "fan_period"
    };
    esp_timer_create(&period_args, &period_timer);

    fan_start_cb(NULL);
    esp_timer_start_periodic(period_timer, PERIOD_US);

    ESP_LOGI(TAG, "Scheduler started. Period: %llu us, Run: %llu us", PERIOD_US, RUN_DURATION_US);
}