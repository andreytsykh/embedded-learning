#include "app_config.h"
#include "bme280_sensor.h"
#include "mqtt_telemetry.h"
#include "ssd1306.h"
#include <math.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

i2c_master_dev_handle_t ssd1306_dev_handle;

static bool display_telemetry(const telemetry_view_t *view)
{
    char temperature[12] = "--", pressure[12] = "--", humidity[12] = "--";
    char line[32];
    if (isfinite(view->temperature))
        snprintf(temperature, sizeof(temperature), "%.2f", (double)view->temperature);
    if (isfinite(view->pressure))
        snprintf(pressure, sizeof(pressure), "%.2f", (double)view->pressure);
    if (isfinite(view->humidity))
        snprintf(humidity, sizeof(humidity), "%.2f", (double)view->humidity);
    ssd1306_clear();
    ssd1306_draw_string(0, 0, APP_MQTT_GROUP);
    ssd1306_draw_string(0, 1, view->source);
    snprintf(line, sizeof(line), "T%s", temperature);
    ssd1306_draw_string(0, 3, line);
    snprintf(line, sizeof(line), "P%s", pressure);
    ssd1306_draw_string(0, 4, line);
    snprintf(line, sizeof(line), "H%s%%", humidity);
    ssd1306_draw_string(0, 5, line);
    return ssd1306_update();
}

void app_main(void)
{
    i2c_master_bus_handle_t bus;
    i2c_master_bus_config_t config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = APP_I2C_SDA,
        .scl_io_num = APP_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&config, &bus));
    i2c_device_config_t oled = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = APP_OLED_ADDRESS,
        .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &oled, &ssd1306_dev_handle));
    ESP_ERROR_CHECK(ssd1306_init() ? ESP_OK : ESP_FAIL);
    ssd1306_draw_string(0, 0, APP_MQTT_GROUP);
    ssd1306_draw_string(0, 2, "Waiting for Wi-Fi");
    ESP_ERROR_CHECK(ssd1306_update() ? ESP_OK : ESP_FAIL);

    bme280_sensor_t sensor = {0};
    esp_err_t err = bme280_sensor_init(&sensor, bus, 0x76);
    if (err == ESP_ERR_NOT_FOUND) err = bme280_sensor_init(&sensor, bus, 0x77);
    ESP_ERROR_CHECK(err);
    mqtt_telemetry_start();
    ssd1306_clear();
    ssd1306_draw_string(0, 0, APP_MQTT_GROUP);
    ssd1306_draw_string(0, 2, "Waiting for MQTT");
    ESP_ERROR_CHECK(ssd1306_update() ? ESP_OK : ESP_FAIL);

    TickType_t previous = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(APP_SAMPLE_PERIOD_MS);
    configASSERT(period > 0);
    while (true) {
        bme280_reading_t reading;
        err = bme280_sensor_read(&sensor, &reading);
        if (err == ESP_OK) {
            if (mqtt_telemetry_connected()) {
                err = mqtt_telemetry_publish(&reading);
                if (err != ESP_OK) ESP_LOGW("APP", "Publish: %s", esp_err_to_name(err));
            }
        } else {
            ESP_LOGE("APP", "Sensor: %s", esp_err_to_name(err));
        }

        telemetry_view_t view;
        if (mqtt_telemetry_get_view(&view) && !display_telemetry(&view))
            ESP_LOGW("APP", "OLED update failed");
        if (xTaskDelayUntil(&previous, period) == pdFALSE)
            previous = xTaskGetTickCount();
    }
}
