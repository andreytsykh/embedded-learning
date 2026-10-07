idf_component_register(
    SRCS "src/main.c" "src/bme280_sensor.c" "src/mqtt_telemetry.c"
         "src/ssd1306.c" "src/bosch/bme280.c"
    INCLUDE_DIRS "include" "include/bosch"
    REQUIRES esp_driver_gpio esp_driver_i2c esp_wifi esp_event esp_netif nvs_flash mqtt
)
target_compile_definitions(${COMPONENT_LIB} PUBLIC BME280_DOUBLE_ENABLE)
