idf_component_register(SRCS "src/main.c"
                       INCLUDE_DIRS "." "include"
                       REQUIRES esp_driver_ledc esp_driver_gpio esp_adc esp_timer
                       PRIV_REQUIRES esp_driver_ledc esp_driver_gpio esp_adc)