/*
 * Copy this file into the STM32CubeMX project. It expects SPI1 configured as
 * a hardware-NSS slave, 8-bit, MSB first, CPOL low, CPHA first edge.
 */
#include "spi_command_slave.h"

#include <stdint.h>

#include "main.h"

#define COMMAND_FRAME_SIZE  4U
#define COMMAND_FRAME_START 0xA5U
#define COMMAND_FRAME_END   0x5AU
#define COMMAND_OPEN        0x01U
#define COMMAND_CLOSE       0x02U

extern SPI_HandleTypeDef hspi1;

static uint8_t rx_frame[COMMAND_FRAME_SIZE];

static void arm_next_receive(void)
{
    (void)HAL_SPI_Receive_IT(&hspi1, rx_frame, COMMAND_FRAME_SIZE);
}

void spi_command_slave_start(void)
{
    arm_next_receive();
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance != SPI1) {
        return;
    }

    const uint8_t command = rx_frame[1];
    const uint8_t inverse = rx_frame[2];
    const uint8_t valid_frame =
        (rx_frame[0] == COMMAND_FRAME_START) &&
        (rx_frame[3] == COMMAND_FRAME_END) &&
        ((uint8_t)~command == inverse);

    if (valid_frame) {
        if (command == COMMAND_OPEN) {
            safe_actuator_open();
        } else if (command == COMMAND_CLOSE) {
            safe_actuator_close();
        }
    }

    arm_next_receive();
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1) {
        arm_next_receive();
    }
}
