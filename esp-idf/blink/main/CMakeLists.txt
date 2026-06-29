idf_component_register(SRCS "src/button.c" "src/main.c"
                       INCLUDE_DIRS "." "include"
                    REQUIRES esp_driver_gpio esp_driver_uart  esp_timer)
