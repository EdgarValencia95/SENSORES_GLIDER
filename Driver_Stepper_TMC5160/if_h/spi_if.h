#ifndef SPI_IF_H_
#define SPI_IF_H_

#include <stdint.h>

///////////////////////////////////////////////////Enumeraciones//////////////////////////////////////////////////

typedef enum
{
    SPI_IF_OK = 0,
    SPI_IF_ERROR = 1,
    SPI_IF_TIMEOUT = 2

} spi_if_status_t;

////////////////////////////////////////////////////Estructura////////////////////////////////////////////////////

typedef struct
{
    void (*init)(void);

    spi_if_status_t (*transfer_byte)(uint8_t tx, uint8_t *rx);

    spi_if_status_t (*transfer)(uint8_t *tx,
                                uint8_t *rx,
                                uint16_t len);

    spi_if_status_t (*send_then_read_byte)(uint8_t tx,
                                           uint8_t dummy,
                                           uint8_t *response);

    void (*cs_low)(void);

    void (*cs_high)(void);

} spi_if_t;

extern const spi_if_t spi1_STM32;

#endif /* SPI_IF_H_ */
