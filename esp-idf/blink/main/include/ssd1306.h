#pragma once

#include <stdint.h>
#include <stdbool.h>

#define SSD1306_I2C_ADDR 0x3C

#define SSD1306_WIDTH    128
#define SSD1306_HEIGHT   64
#define SSD1306_PAGES    8

bool ssd1306_init(void);
void ssd1306_clear(void);
void ssd1306_update(void);

void ssd1306_draw_char(uint8_t x, uint8_t page, char c);
void ssd1306_draw_string(uint8_t x, uint8_t page, const char *str);