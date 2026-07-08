#include "ds1307.h"

#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "DS1307";

extern i2c_master_dev_handle_t ds1307_dev_handle;

static uint8_t bcd_to_dec(uint8_t bcd)
{
    return ((bcd >> 4U) * 10U) + (bcd & 0x0FU);
}

bool ds1307_read_datetime(ds1307_datetime_t *dt)
{
    if (dt == NULL) {
        return false;
    }

    uint8_t reg = 0x00;
    uint8_t data[7] = {0};

    esp_err_t err = i2c_master_transmit_receive(
        ds1307_dev_handle,
        &reg,
        1,
        data,
        sizeof(data),
        1000
    );

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Read failed: %s", esp_err_to_name(err));
        return false;
    }

    dt->seconds = bcd_to_dec(data[0] & 0x7FU);
    dt->minutes = bcd_to_dec(data[1] & 0x7FU);
    dt->hours   = bcd_to_dec(data[2] & 0x3FU);

    dt->day_of_week = bcd_to_dec(data[3] & 0x07U);
    dt->day         = bcd_to_dec(data[4] & 0x3FU);
    dt->month       = bcd_to_dec(data[5] & 0x1FU);
    dt->year        = 2000U + bcd_to_dec(data[6]);

    ESP_LOGI(
        TAG,
        "RTC: %02u:%02u:%02u  %02u.%02u.%04u  weekday=%u",
        dt->hours,
        dt->minutes,
        dt->seconds,
        dt->day,
        dt->month,
        dt->year,
        dt->day_of_week
    );

    return true;
}

static uint8_t dec_to_bcd(uint8_t dec)
{
    return ((dec / 10U) << 4U) | (dec % 10U);
}

bool ds1307_set_datetime(const ds1307_datetime_t *dt)
{
    if (dt == NULL) {
        return false;
    }

    uint8_t data[8];

    data[0] = 0x00; // start register

    data[1] = dec_to_bcd(dt->seconds) & 0x7FU;
    data[2] = dec_to_bcd(dt->minutes);
    data[3] = dec_to_bcd(dt->hours);        // 24h mode
    data[4] = dec_to_bcd(dt->day_of_week);  // 1..7
    data[5] = dec_to_bcd(dt->day);
    data[6] = dec_to_bcd(dt->month);
    data[7] = dec_to_bcd((uint8_t)(dt->year - 2000U));

    esp_err_t err = i2c_master_transmit(
        ds1307_dev_handle,
        data,
        sizeof(data),
        1000
    );

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Set datetime failed: %s", esp_err_to_name(err));
        return false;
    }

    ESP_LOGI(
        TAG,
        "RTC set: %02u:%02u:%02u  %02u.%02u.%04u  weekday=%u",
        dt->hours,
        dt->minutes,
        dt->seconds,
        dt->day,
        dt->month,
        dt->year,
        dt->day_of_week
    );

    return true;
}