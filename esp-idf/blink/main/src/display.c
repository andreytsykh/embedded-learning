#include "display.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_log.h"
#include "ssd1306.h"

#include "main.h"

#define SAFE_CODE_LENGTH 4U

static const char *TAG = "oled_display";

static i2c_master_bus_handle_t i2c_bus;
static ssd1306_handle_t display;
static char code[SAFE_CODE_LENGTH + 1U] = "____";
static uint8_t previous_index;
static uint8_t attempts_left;

static esp_err_t render_code(void)
{
    char attempts_text[20];
    (void)snprintf(
        attempts_text,
        sizeof(attempts_text),
        "TRIES LEFT: %u",
        (unsigned int)attempts_left
    );

    ESP_RETURN_ON_ERROR(ssd1306_clear(display), TAG, "clear display");
    ESP_RETURN_ON_ERROR(
        ssd1306_draw_text(display, 34, 2, "ENTER CODE", true),
        TAG,
        "draw title"
    );
    ESP_RETURN_ON_ERROR(
        ssd1306_draw_text_scaled(display, 28, 20, code, true, 3),
        TAG,
        "draw code"
    );
    ESP_RETURN_ON_ERROR(
        ssd1306_draw_text(display, 25, 54, attempts_text, true),
        TAG,
        "draw attempts"
    );
    return ssd1306_display(display);
}

esp_err_t oled_display_init(void)
{
    if (display != NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    const i2c_master_bus_config_t bus_config = {
        .i2c_port = OLED_I2C_PORT,
        .sda_io_num = OLED_SDA_GPIO,
        .scl_io_num = OLED_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = false,
        },
    };

    ESP_RETURN_ON_ERROR(
        i2c_new_master_bus(&bus_config, &i2c_bus),
        TAG,
        "create I2C bus"
    );

    const ssd1306_config_t display_config = {
        .bus = SSD1306_I2C,
        .width = OLED_WIDTH,
        .height = OLED_HEIGHT,
        .iface.i2c = {
            .port = OLED_I2C_PORT,
            .addr = OLED_I2C_ADDRESS,
            .rst_gpio = GPIO_NUM_NC,
        },
    };

    esp_err_t err = ssd1306_new_i2c(&display_config, &display);
    if (err != ESP_OK) {
        (void)i2c_del_master_bus(i2c_bus);
        i2c_bus = NULL;
        display = NULL;
        ESP_LOGE(TAG, "SSD1306 initialization failed: %s", esp_err_to_name(err));
        return err;
    }

    memcpy(code, "____", sizeof(code));
    previous_index = 0U;
    attempts_left = 0U;
    return ESP_OK;
}

void oled_display_show_code(
    uint8_t index,
    uint8_t digit,
    uint8_t attempts_remaining
)
{
    if (display == NULL) {
        ESP_LOGW(TAG, "display is not initialized");
        return;
    }

    if ((index >= SAFE_CODE_LENGTH) || (digit > 9U)) {
        ESP_LOGW(TAG, "invalid code digit: index=%u, digit=%u", index, digit);
        return;
    }

    if (index < previous_index) {
        memcpy(code, "____", sizeof(code));
    }

    code[index] = (char)('0' + digit);
    previous_index = index;
    attempts_left = attempts_remaining;

    esp_err_t err = render_code();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "code rendering failed: %s", esp_err_to_name(err));
    }
}

void oled_display_show_message(const char *message)
{
    if ((display == NULL) || (message == NULL)) {
        return;
    }

    const int scale = 2;
    const int glyph_width = 6;
    const int display_width = (int)OLED_WIDTH;
    const int display_height = (int)OLED_HEIGHT;
    const int text_width = (int)strlen(message) * glyph_width * scale;
    const int x = (text_width < display_width)
        ? ((display_width - text_width) / 2)
        : 0;
    const int y = (display_height - (7 * scale)) / 2;

    esp_err_t err = ssd1306_clear(display);
    if (err == ESP_OK) {
        if (text_width < display_width) {
            err = ssd1306_draw_text_scaled(
                display,
                x,
                y,
                message,
                true,
                scale
            );
        } else {
            err = ssd1306_draw_text_wrapped(
                display,
                0,
                0,
                OLED_WIDTH,
                OLED_HEIGHT,
                message,
                true
            );
        }
    }
    if (err == ESP_OK) {
        err = ssd1306_display(display);
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "message rendering failed: %s", esp_err_to_name(err));
    }
}
