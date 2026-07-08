idf_component_register(SRCS "src/main.c" "src/ssd1306.c" "src/ds1307.c"
                       INCLUDE_DIRS "." "include"
                       REQUIRES driver esp_driver_i2c) 
