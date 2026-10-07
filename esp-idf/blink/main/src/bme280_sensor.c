#include "bme280_sensor.h"
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifndef BME280_DOUBLE_ENABLE
#error "Enable BME280_DOUBLE_ENABLE in CMakeLists.txt"
#endif

#define IO_TIMEOUT_MS 100

static BME280_INTF_RET_TYPE bus_read(
    uint8_t reg, uint8_t *data, uint32_t len, void *context)
{
    bme280_sensor_t *s = context;

    s->last_io_error = i2c_master_transmit_receive(
        s->i2c, &reg, 1, data, len, IO_TIMEOUT_MS);

    return s->last_io_error == ESP_OK ? 0 : -1;
}

static BME280_INTF_RET_TYPE bus_write(
    uint8_t reg, const uint8_t *data, uint32_t len, void *context)
{
    bme280_sensor_t *s = context;
    uint8_t buffer[257];

    if (!data || len == 0 || len > 256) {
        s->last_io_error = ESP_ERR_INVALID_ARG;
        return -1;
    }

    buffer[0] = reg;
    memcpy(buffer + 1, data, len);

    s->last_io_error = i2c_master_transmit(
        s->i2c, buffer, len + 1, IO_TIMEOUT_MS);

    return s->last_io_error == ESP_OK ? 0 : -1;
}

static void delay_us(uint32_t us, void *context)
{
    (void)context;

    TickType_t ticks =
        ((uint64_t)us * configTICK_RATE_HZ + 999999) / 1000000;

    vTaskDelay(ticks + 1);
}

esp_err_t bme280_sensor_init(
    bme280_sensor_t *s, i2c_master_bus_handle_t bus, uint8_t address)
{
    if (!s || !bus || (address != 0x76 && address != 0x77)) {
        return ESP_ERR_INVALID_ARG;
    }

    if (s->i2c || s->initialized) return ESP_ERR_INVALID_STATE;

    delay_us(10000, NULL);

    esp_err_t err = i2c_master_probe(bus, address, IO_TIMEOUT_MS);
    if (err != ESP_OK) return err;

    i2c_device_config_t config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = 100000,
    };

    err = i2c_master_bus_add_device(bus, &config, &s->i2c);
    if (err != ESP_OK) return err;

    s->dev.intf = BME280_I2C_INTF;
    s->dev.intf_ptr = s;
    s->dev.read = bus_read;
    s->dev.write = bus_write;
    s->dev.delay_us = delay_us;

    struct bme280_settings settings = {
        .osr_h = BME280_OVERSAMPLING_1X,
        .osr_p = BME280_OVERSAMPLING_1X,
        .osr_t = BME280_OVERSAMPLING_1X,
        .filter = BME280_FILTER_COEFF_OFF,
        .standby_time = BME280_STANDBY_TIME_0_5_MS,
    };

    if (bme280_init(&s->dev) != BME280_OK) goto fail;

    if (bme280_set_sensor_settings(
            BME280_SEL_ALL_SETTINGS, &settings, &s->dev)
        != BME280_OK) goto fail;

    if (bme280_cal_meas_delay(&s->measurement_us, &settings)
        != BME280_OK) goto fail;

    s->initialized = true;
    return ESP_OK;

fail:
    (void)bme280_sensor_deinit(s);
    return ESP_FAIL;
}

esp_err_t bme280_sensor_read(
    bme280_sensor_t *s, bme280_reading_t *out)
{
    if (!s || !out) return ESP_ERR_INVALID_ARG;
    if (!s->initialized) return ESP_ERR_INVALID_STATE;

    if (bme280_set_sensor_mode(BME280_POWERMODE_FORCED, &s->dev)
        != BME280_OK) return ESP_FAIL;

    delay_us(s->measurement_us + 1000, NULL);

    struct bme280_data data;

    if (bme280_get_sensor_data(BME280_ALL, &data, &s->dev)
        != BME280_OK) return ESP_FAIL;

    out->temperature_c = data.temperature;
    out->humidity_pct = data.humidity;
    out->pressure_hpa = data.pressure / 100.0;

    return ESP_OK;
}

esp_err_t bme280_sensor_deinit(bme280_sensor_t *s)
{
    if (!s) return ESP_ERR_INVALID_ARG;

    if (s->i2c) {
        esp_err_t err = i2c_master_bus_rm_device(s->i2c);
        if (err != ESP_OK) return err;
    }

    memset(s, 0, sizeof(*s));
    return ESP_OK;
}