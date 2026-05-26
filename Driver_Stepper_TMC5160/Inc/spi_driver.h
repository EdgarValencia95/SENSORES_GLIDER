#ifndef SPI_DRIVER_H_
#define SPI_DRIVER_H_

#include <stdint.h>
#include "spi_if.h"

/////////////////////////////////////////Declaracion Funciones///////////////////////////////////////////////////

void spi1_pa5_pa6_pa7_init_mode3_prescaler32(void);

spi_if_status_t spi1_transfer_byte(uint8_t tx, uint8_t *rx);

spi_if_status_t spi1_transfer(uint8_t *tx, uint8_t *rx, uint16_t len);

spi_if_status_t spi1_send_then_read_byte(uint8_t tx,
                                         uint8_t dummy,
                                         uint8_t *response);

void spi1_cs_low(void);

void spi1_cs_high(void);

#endif /* SPI_DRIVER_H_ */
