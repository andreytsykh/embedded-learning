#include "spi_command_slave.h"

extern SPI_HandleTypeDef hspi1;

// ---- Протокол кадру ----
#define FRAME_LEN      4
#define FRAME_START    0xA5
#define FRAME_END      0x5A
#define CMD_OPEN       0x01
#define CMD_CLOSE      0x02

static uint8_t rx_frame[FRAME_LEN];
static uint8_t tx_dummy[FRAME_LEN] = {0};

// debug
volatile HAL_StatusTypeDef dbg_spi_start_status = HAL_OK;
volatile uint32_t dbg_spi_started = 0;
volatile uint32_t dbg_spi_complete = 0;
volatile uint32_t dbg_spi_errors = 0;
volatile uint32_t dbg_spi_error_code = HAL_SPI_ERROR_NONE;
volatile uint32_t dbg_spi_invalid_frame = 0;
volatile uint32_t dbg_spi_bad_checksum = 0;
volatile uint32_t dbg_spi_unknown_command = 0;
volatile uint32_t dbg_cs_falling = 0;
volatile uint32_t dbg_cs_rising = 0;
volatile uint8_t dbg_last_frame[FRAME_LEN] = {0};

static void restart_listen(void)
{
    dbg_spi_start_status = HAL_SPI_TransmitReceive_IT(
        &hspi1, tx_dummy, rx_frame, FRAME_LEN);

    if (dbg_spi_start_status == HAL_OK) {
        dbg_spi_started++;
    }
}

void spi_command_slave_start(void)
{
    restart_listen();
}

static void process_frame(uint8_t *frame)
{
    if (frame[0] != FRAME_START || frame[3] != FRAME_END) {
        dbg_spi_invalid_frame++;
        return;
    }

    uint8_t cmd     = frame[1];
    uint8_t cmd_inv = frame[2];

    if (cmd_inv != (uint8_t)(~cmd)) {
        dbg_spi_bad_checksum++;
        return;
    }

    switch (cmd) {
        case CMD_OPEN:
            on_open();
            break;
        case CMD_CLOSE:
            on_close();
            break;
        default:
            dbg_spi_unknown_command++;
            break;
    }
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance != SPI1) {
        return;
    }

    for (uint32_t i = 0; i < FRAME_LEN; i++) {
        dbg_last_frame[i] = rx_frame[i];
    }

    dbg_spi_complete++;
    process_frame(rx_frame);
    restart_listen();
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance != SPI1) {
        return;
    }

    dbg_spi_errors++;
    dbg_spi_error_code = hspi->ErrorCode;
    __HAL_SPI_CLEAR_OVRFLAG(hspi); 
    //__HAL_SPI_CLEAR_OVRFLAG(&hspi1) очищає помилку SPI OVR — overrun.
    // OVR виникає, коли SPI отримав новий байт раніше, ніж програма прочитала попередній із регістра DR
    restart_listen();
}
