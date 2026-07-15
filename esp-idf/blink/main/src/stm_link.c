#include "stm_link.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_log.h"

#include "main.h"

#define STM_FRAME_SIZE       4U
#define STM_FRAME_START      0xA5U
#define STM_FRAME_END        0x5AU
#define STM_SPI_CLOCK_HZ     100000

static const char *TAG = "stm_link";

static spi_device_handle_t stm_device;
static bool initialized;

esp_err_t stm_link_init(void)
{
    if (initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    const spi_bus_config_t bus_config = {
        .mosi_io_num = STM_SPI_MOSI_GPIO,
        .miso_io_num = STM_SPI_MISO_GPIO,
        .sclk_io_num = STM_SPI_SCLK_GPIO,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = STM_FRAME_SIZE,
    };

    ESP_RETURN_ON_ERROR(
        spi_bus_initialize(STM_SPI_HOST, &bus_config, SPI_DMA_DISABLED),
        TAG,
        "initialize SPI bus"
    );

    const spi_device_interface_config_t device_config = {
        .clock_speed_hz = STM_SPI_CLOCK_HZ,
        .mode = 0,
        .spics_io_num = STM_SPI_CS_GPIO,
        .queue_size = 1,
    };

    esp_err_t err = spi_bus_add_device(
        STM_SPI_HOST,
        &device_config,
        &stm_device
    );
    if (err != ESP_OK) {
        (void)spi_bus_free(STM_SPI_HOST);
        return err;
    }

    initialized = true;
    ESP_LOGI(
        TAG,
        "STM SPI initialized: SCLK=%d, MOSI=%d, MISO=%d, CS=%d",
        STM_SPI_SCLK_GPIO,
        STM_SPI_MOSI_GPIO,
        STM_SPI_MISO_GPIO,
        STM_SPI_CS_GPIO
    );
    return ESP_OK;
}

esp_err_t stm_send_command(stm_command_t command)
{
    if (!initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if ((command != STM_COMMAND_OPEN) &&
        (command != STM_COMMAND_CLOSE)) {
        return ESP_ERR_INVALID_ARG;
    }

    const uint8_t tx_data[STM_FRAME_SIZE] = {
        STM_FRAME_START,
        (uint8_t)command,
        (uint8_t)~command,
        STM_FRAME_END,
    };
    uint8_t rx_data[STM_FRAME_SIZE] = { 0U };

    spi_transaction_t transaction = {
        .length = STM_FRAME_SIZE * 8U,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    esp_err_t err = spi_device_transmit(stm_device, &transaction);
    if (err == ESP_OK) {
        ESP_LOGI(
            TAG,
            "Command sent: %s",
            (command == STM_COMMAND_OPEN) ? "OPEN" : "CLOSE"
        );
    }
    return err;
}
