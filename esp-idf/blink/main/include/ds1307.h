#pragma once

#include <stdint.h>
#include <stdbool.h>

#define DS1307_I2C_ADDR 0x68

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;

    uint8_t day_of_week; // 1..7
    uint8_t day;
    uint8_t month;
    uint16_t year;
} ds1307_datetime_t;

bool ds1307_read_datetime(ds1307_datetime_t *dt);
bool ds1307_set_datetime(const ds1307_datetime_t *dt);