#ifndef SPI_COMMAND_SLAVE_H
#define SPI_COMMAND_SLAVE_H

#include "stm32f4xx_hal.h" 

void spi_command_slave_start(void);

extern void on_open(void);
extern void on_close(void);

#endif // SPI_COMMAND_SLAVE_H