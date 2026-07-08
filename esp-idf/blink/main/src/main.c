#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_err.h"

#include "ssd1306.h"
#include "ds1307.h"

#define I2C_SDA_PIN        7
#define I2C_SCL_PIN        6
#define I2C_FREQ_HZ        100000

static const char *TAG = "MAIN";

i2c_master_bus_handle_t i2c_bus_handle = NULL;
i2c_master_dev_handle_t ssd1306_dev_handle = NULL;
i2c_master_dev_handle_t ds1307_dev_handle = NULL;

static void i2c_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = true,
        },
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus_handle));

    i2c_device_config_t ssd1306_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SSD1306_I2C_ADDR,
        .scl_speed_hz = I2C_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(
        i2c_bus_handle,
        &ssd1306_config,
        &ssd1306_dev_handle
    ));

    i2c_device_config_t ds1307_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = DS1307_I2C_ADDR,
        .scl_speed_hz = I2C_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(
        i2c_bus_handle,
        &ds1307_config,
        &ds1307_dev_handle
    ));

    ESP_LOGI(TAG, "I2C initialized");
    ESP_LOGI(TAG, "SSD1306 addr: 0x%02X", SSD1306_I2C_ADDR);
    ESP_LOGI(TAG, "DS1307 addr: 0x%02X", DS1307_I2C_ADDR);
}

static const char *weekday_to_str(uint8_t weekday)
{
    switch (weekday) {
        case 1: return "Sun";
        case 2: return "Mon";
        case 3: return "Tue";
        case 4: return "Wed";
        case 5: return "Thu";
        case 6: return "Fri";
        case 7: return "Sat";
        default: return "---";
    }
}

static void display_datetime(const ds1307_datetime_t *dt)
{
    char time_str[16];
    char date_str[24];

    snprintf(
        time_str,
        sizeof(time_str),
        "%02u:%02u:%02u",
        dt->hours,
        dt->minutes,
        dt->seconds
    );

    snprintf(
        date_str,
        sizeof(date_str),
        "%s %02u.%02u.%04u",
        weekday_to_str(dt->day_of_week),
        dt->day,
        dt->month,
        dt->year
    );

    ESP_LOGI(TAG, "Display: %s | %s", time_str, date_str);

    ssd1306_clear();
    ssd1306_draw_string(22, 2, time_str);
    ssd1306_draw_string(8, 4, date_str);
    ssd1306_update();
}

static void i2c_scan(void)
{
    ESP_LOGI(TAG, "Scanning I2C bus...");

    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_device_config_t dev_config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = addr,
            .scl_speed_hz = I2C_FREQ_HZ,
        };

        i2c_master_dev_handle_t dev_handle = NULL;

        esp_err_t err = i2c_master_bus_add_device(
            i2c_bus_handle,
            &dev_config,
            &dev_handle
        );

        if (err != ESP_OK) {
            continue;
        }

        err = i2c_master_probe(
            i2c_bus_handle,
            addr,
            pdMS_TO_TICKS(50)
        );

        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Found I2C device at 0x%02X", addr);
        }

        i2c_master_bus_rm_device(dev_handle);
    }

    ESP_LOGI(TAG, "I2C scan done");
}

void app_main(void)
{
    i2c_init();
    i2c_scan();
    ds1307_datetime_t initial_time = {
    .seconds = 00,
    .minutes = 20,
    .hours = 18,

    .day_of_week = 7,
    .day = 5,
    .month = 7,
    .year = 2026,
    };

    // ds1307_set_datetime(&initial_time);

    if (!ssd1306_init()) {
        ESP_LOGE(TAG, "SSD1306 init failed");
        return;
    }

    while (1) {
        ds1307_datetime_t now;

        if (ds1307_read_datetime(&now)) {
            display_datetime(&now);
        } else {
            ESP_LOGE(TAG, "DS1307 read failed");

            ssd1306_clear();
            ssd1306_draw_string(0, 2, "RTC ERROR");
            ssd1306_update();
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

