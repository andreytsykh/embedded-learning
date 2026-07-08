#include "ssd1306.h"

#include <string.h>

#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "esp_err.h"

#include "esp_log.h"

static const char *TAG = "SSD1306";

extern i2c_master_dev_handle_t ssd1306_dev_handle;

static uint8_t framebuffer[SSD1306_WIDTH * SSD1306_PAGES];

static bool ssd1306_write_command(uint8_t cmd)
{
    uint8_t data[2] = {
        0x00,
        cmd
    };

    esp_err_t err = i2c_master_transmit(
        ssd1306_dev_handle,
        data,
        sizeof(data),
        1000
    );

    return err == ESP_OK;
}

static bool ssd1306_write_commands(const uint8_t *cmds, uint8_t len)
{
    uint8_t data[32];

    if (len > 31U) {
        ESP_LOGE(TAG, "Command packet too large: %u", len);
        return false;
    }

    data[0] = 0x00;
    memcpy(&data[1], cmds, len);

    esp_err_t err = i2c_master_transmit(
        ssd1306_dev_handle,
        data,
        len + 1U,
        1000
    );

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Command transmit failed: %s", esp_err_to_name(err));
        return false;
    }

    return true;
}

static bool ssd1306_write_data(const uint8_t *data_in, uint16_t len)
{
    uint8_t data[129];

    while (len > 0U) {
        uint8_t chunk = (len > 128U) ? 128U : (uint8_t)len;

        data[0] = 0x40;
        memcpy(&data[1], data_in, chunk);

        esp_err_t err = i2c_master_transmit(
            ssd1306_dev_handle,
            data,
            chunk + 1U,
            1000
        );

        if (err != ESP_OK) {
            return false;
        }

        data_in += chunk;
        len -= chunk;
    }

    return true;
}

bool ssd1306_init(void)
{
    const uint8_t init_cmds[] = {
        0xAE,

        0xD5, 0x80,
        0xA8, 0x3F,
        0xD3, 0x00,
        0x40,

        0x8D, 0x14,

        0x20, 0x00,

        0xA1,
        0xC8,

        0xDA, 0x12,
        0x81, 0x7F,
        0xD9, 0xF1,
        0xDB, 0x40,

        0xA4,
        0xA6,
        0x2E,

        0xAF
    };

    if (!ssd1306_write_commands(init_cmds, sizeof(init_cmds))) {
        return false;
    }

    ssd1306_clear();
    ssd1306_update();

    return true;
}

void ssd1306_clear(void)
{
    memset(framebuffer, 0x00, sizeof(framebuffer));
}

void ssd1306_update(void)
{
    const uint8_t cmds[] = {
        0x21, 0x00, 0x7F,
        0x22, 0x00, 0x07
    };

    ssd1306_write_commands(cmds, sizeof(cmds));
    ssd1306_write_data(framebuffer, sizeof(framebuffer));
}

static const uint8_t *font_get_char(char c)
{
    static const uint8_t space[5] = {0x00,0x00,0x00,0x00,0x00};

    static const uint8_t n0[5] = {0x3E,0x51,0x49,0x45,0x3E};
    static const uint8_t n1[5] = {0x00,0x42,0x7F,0x40,0x00};
    static const uint8_t n2[5] = {0x42,0x61,0x51,0x49,0x46};
    static const uint8_t n3[5] = {0x21,0x41,0x45,0x4B,0x31};
    static const uint8_t n4[5] = {0x18,0x14,0x12,0x7F,0x10};
    static const uint8_t n5[5] = {0x27,0x45,0x45,0x45,0x39};
    static const uint8_t n6[5] = {0x3C,0x4A,0x49,0x49,0x30};
    static const uint8_t n7[5] = {0x01,0x71,0x09,0x05,0x03};
    static const uint8_t n8[5] = {0x36,0x49,0x49,0x49,0x36};
    static const uint8_t n9[5] = {0x06,0x49,0x49,0x29,0x1E};

    static const uint8_t colon[5] = {0x00,0x36,0x36,0x00,0x00};
    static const uint8_t dot[5]   = {0x00,0x60,0x60,0x00,0x00};

    static const uint8_t A[5] = {0x7E,0x11,0x11,0x11,0x7E};
    static const uint8_t D[5] = {0x7F,0x41,0x41,0x22,0x1C};
    static const uint8_t F[5] = {0x7F,0x09,0x09,0x09,0x01};
    static const uint8_t M[5] = {0x7F,0x02,0x0C,0x02,0x7F};
    static const uint8_t S[5] = {0x46,0x49,0x49,0x49,0x31};
    static const uint8_t T[5] = {0x01,0x01,0x7F,0x01,0x01};
    static const uint8_t W[5] = {0x7F,0x20,0x18,0x20,0x7F};

    static const uint8_t a[5] = {0x20,0x54,0x54,0x54,0x78};
    static const uint8_t d[5] = {0x38,0x44,0x44,0x48,0x7F};
    static const uint8_t e[5] = {0x38,0x54,0x54,0x54,0x18};
    static const uint8_t h[5] = {0x7F,0x08,0x04,0x04,0x78};
    static const uint8_t i[5] = {0x00,0x44,0x7D,0x40,0x00};
    static const uint8_t n[5] = {0x7C,0x08,0x04,0x04,0x78};
    static const uint8_t o[5] = {0x38,0x44,0x44,0x44,0x38};
    static const uint8_t r[5] = {0x7C,0x08,0x04,0x04,0x08};
    static const uint8_t t[5] = {0x04,0x3F,0x44,0x40,0x20};
    static const uint8_t u[5] = {0x3C,0x40,0x40,0x20,0x7C};

    switch (c) {
        case '0': return n0;
        case '1': return n1;
        case '2': return n2;
        case '3': return n3;
        case '4': return n4;
        case '5': return n5;
        case '6': return n6;
        case '7': return n7;
        case '8': return n8;
        case '9': return n9;

        case ':': return colon;
        case '.': return dot;
        case ' ': return space;

        case 'A': return A;
        case 'D': return D;
        case 'F': return F;
        case 'M': return M;
        case 'S': return S;
        case 'T': return T;
        case 'W': return W;

        case 'a': return a;
        case 'd': return d;
        case 'e': return e;
        case 'h': return h;
        case 'i': return i;
        case 'n': return n;
        case 'o': return o;
        case 'r': return r;
        case 't': return t;
        case 'u': return u;

        default: return space;
    }
}

void ssd1306_draw_char(uint8_t x, uint8_t page, char c)
{
    if (x >= SSD1306_WIDTH || page >= SSD1306_PAGES) {
        return;
    }

    const uint8_t *bitmap = font_get_char(c);

    for (uint8_t i = 0; i < 5U; i++) {
        if ((x + i) < SSD1306_WIDTH) {
            framebuffer[(page * SSD1306_WIDTH) + x + i] = bitmap[i];
        }
    }

    if ((x + 5U) < SSD1306_WIDTH) {
        framebuffer[(page * SSD1306_WIDTH) + x + 5U] = 0x00;
    }
}

void ssd1306_draw_string(uint8_t x, uint8_t page, const char *str)
{
    if (str == NULL) {
        return;
    }

    while (*str != '\0') {
        ssd1306_draw_char(x, page, *str);
        x += 6U;

        if (x >= SSD1306_WIDTH) {
            break;
        }

        str++;
    }
}