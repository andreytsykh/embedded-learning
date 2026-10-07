#pragma once
#include <stdbool.h>
#include "bme280_sensor.h"

typedef struct {
    char source[32];
    float temperature;
    float humidity;
    float pressure;
} telemetry_view_t;

void mqtt_telemetry_start(void);
bool mqtt_telemetry_connected(void);
esp_err_t mqtt_telemetry_publish(const bme280_reading_t *data);
bool mqtt_telemetry_get_view(telemetry_view_t *out);
