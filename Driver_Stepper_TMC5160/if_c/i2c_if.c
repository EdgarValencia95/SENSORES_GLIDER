#include "i2c_if.h"
#include "i2c_driver.h"

const i2c_if_t i2c1_STM32 = {
    .init_100khz = i2c1_pb6_pb7_init_100khz,
    .mem_write = i2c1_mem_write,
    .mem_read = i2c1_mem_read
};
