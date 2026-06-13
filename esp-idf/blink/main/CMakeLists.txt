idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_driver_ledc esp_driver_gpio esp_adc
                       PRIV_REQUIRES esp_driver_ledc esp_driver_gpio esp_adc)
