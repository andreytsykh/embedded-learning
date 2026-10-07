#ifndef BME280_SENSOR_H
#define BME280_SENSOR_H

#include <stdbool.h>
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "bme280.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double temperature_c;
    double pressure_hpa; 
    double humidity_pct; 
} bme280_reading_t;


typedef struct {
    i2c_master_dev_handle_t i2c;
    struct bme280_dev dev;
    uint32_t measurement_us;
    esp_err_t last_io_error;
    bool initialized;
} bme280_sensor_t;


esp_err_t bme280_sensor_init(bme280_sensor_t *sensor,
                            i2c_master_bus_handle_t bus, uint8_t address);

esp_err_t bme280_sensor_read(bme280_sensor_t *sensor, bme280_reading_t *out);

esp_err_t bme280_sensor_deinit(bme280_sensor_t *sensor);

#ifdef __cplusplus
}
#endif
#endif
