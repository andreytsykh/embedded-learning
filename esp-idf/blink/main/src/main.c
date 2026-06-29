#include <stdbool.h>
#include <stdint.h>

#include "button.h"

#include "driver/gpio.h"
#include "driver/uart.h"

#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define TAG "ESP_UART"


#define COMM_UART UART_NUM_1


#define UART_TX_GPIO GPIO_NUM_17
#define UART_RX_GPIO GPIO_NUM_18

#define BUTTON_GPIO GPIO_NUM_10
#define LED_GPIO    GPIO_NUM_6

#define UART_BAUD_RATE 115200

#define BUTTON_DEBOUNCE_US   30000ULL
#define BUTTON_LONG_PRESS_US 1000000ULL

#define COMMAND_TOGGLE_LED ((uint8_t)'T')


static bool send_toggle_requested = false;
static bool long_press_requested = false;

static bool led_state = false;


static void led_set(bool state)
{
    led_state = state;

    ESP_ERROR_CHECK(
        gpio_set_level(
            LED_GPIO,
            led_state ? 1 : 0
        )
    );
}


static void led_toggle(void)
{
    led_set(!led_state);

    ESP_LOGI(
        TAG,
        "ESP32 LED: %s",
        led_state ? "ON" : "OFF"
    );
}


static void gpio_init(void)
{
    const gpio_config_t led_config = {
        .pin_bit_mask = 1ULL << LED_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&led_config));

    led_set(false);
}


static void uart_init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    ESP_ERROR_CHECK(
        uart_driver_install(
            COMM_UART,
            256,
            0,
            0,
            NULL,
            0
        )
    );

    ESP_ERROR_CHECK(
        uart_param_config(
            COMM_UART,
            &uart_config
        )
    );

    ESP_ERROR_CHECK(
        uart_set_pin(
            COMM_UART,
            UART_TX_GPIO,
            UART_RX_GPIO,
            UART_PIN_NO_CHANGE,
            UART_PIN_NO_CHANGE
        )
    );
}


static void uart_send_toggle_command(void)
{
    const uint8_t command = COMMAND_TOGGLE_LED;

    const int written_bytes = uart_write_bytes(
        COMM_UART,
        &command,
        1U
    );

    if (written_bytes == 1) {
        ESP_LOGI(TAG, "Sent 'T' to STM32");
    } else {
        ESP_LOGE(TAG, "UART send failed");
    }
}


static void uart_receive_process(void)
{
    uint8_t received_byte = 0U;

    const int received_length = uart_read_bytes(
        COMM_UART,
        &received_byte,
        1U,
        0U
    );

    if (received_length != 1) {
        return;
    }

    ESP_LOGI(
        TAG,
        "Received: 0x%02X '%c'",
        received_byte,
        (char)received_byte
    );

    if (received_byte == COMMAND_TOGGLE_LED) {
        led_toggle();
    }
}


static void on_button_short_press(void *ctx)
{
    (void)ctx;

    send_toggle_requested = true;
}


static button_t button = {
    .gpio = BUTTON_GPIO,
    .state = BUTTON_IDLE,
    .debounce_us = BUTTON_DEBOUNCE_US,
    .long_press_us = BUTTON_LONG_PRESS_US,
    .on_short_press = on_button_short_press,
    .ctx = NULL
};


void app_main(void)
{
    gpio_init();
    uart_init();

    button_init(&button);

    ESP_LOGI(TAG, "Application started");

    while (true) {
        uart_receive_process();

        if (send_toggle_requested) {
            send_toggle_requested = false;

            uart_send_toggle_command();
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}