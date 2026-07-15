#ifndef MAIN_H
#define MAIN_H

#include "driver/gpio.h"
#include "driver/i2c_master.h"

#define MAIN_TAG "ENCODER_DISPLAY"

/* Code and safe-state indication LEDs. */
#define LED_1_GPIO      GPIO_NUM_11
#define LED_2_GPIO      GPIO_NUM_36
#define LED_3_GPIO      GPIO_NUM_37
#define LED_4_GPIO      GPIO_NUM_12
#define LED_LOCK_GPIO   GPIO_NUM_8
#define LED_UNLOCK_GPIO GPIO_NUM_41

/* Buzzer. */
#define BUZZER_GPIO GPIO_NUM_9

/* Rotary encoder and its push button. */
#define ENCODER_A_GPIO  GPIO_NUM_18
#define ENCODER_B_GPIO  GPIO_NUM_21
#define ENCODER_SW_GPIO GPIO_NUM_10

#define BUTTON_DEBOUNCE_US   50000ULL
#define BUTTON_LONG_PRESS_US 4000000ULL

#define ENCODER_COUNTS_PER_DETENT 4
#define ENCODER_GLITCH_FILTER_NS  1000U

/* SSD1306 OLED connected over I2C. */
#define OLED_I2C_PORT    I2C_NUM_0
#define OLED_SDA_GPIO    GPIO_NUM_40
#define OLED_SCL_GPIO    GPIO_NUM_2
#define OLED_I2C_ADDRESS 0x3CU
#define OLED_WIDTH       128U
#define OLED_HEIGHT      64U

/* STM32 command link over SPI2. */
#define STM_SPI_HOST      SPI2_HOST
#define STM_SPI_SCLK_GPIO GPIO_NUM_4
#define STM_SPI_MOSI_GPIO GPIO_NUM_5
#define STM_SPI_MISO_GPIO GPIO_NUM_6
#define STM_SPI_CS_GPIO   GPIO_NUM_7

#define MAIN_LOOP_DELAY_MS 10U

#endif /* MAIN_H */
