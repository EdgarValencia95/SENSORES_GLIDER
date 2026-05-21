#include "spi_if.h"
#include "spi_driver.h"

const spi_if_t spi1_STM32 = {
    .init = spi1_pa5_pa6_pa7_init_mode3_prescaler32,
    .transfer_byte = spi1_transfer_byte,
    .transfer = spi1_transfer,
    .send_then_read_byte = spi1_send_then_read_byte,
    .cs_low = spi1_cs_low,
    .cs_high = spi1_cs_high
};
