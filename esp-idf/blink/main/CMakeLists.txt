idf_component_register(SRCS src/main.c src/button.c src/melodies.c
                       INCLUDE_DIRS "include"
                       REQUIRES esp_driver_ledc esp_timer esp_driver_gpio)
