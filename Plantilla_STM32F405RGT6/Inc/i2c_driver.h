#ifndef I2C_DRIVER_H_
#define I2C_DRIVER_H_

#include <stdint.h>
#include "i2c_if.h"

void i2c1_pb6_pb7_init_100khz(void);

i2c_if_status_t i2c1_check_device(uint8_t dev_addr_7bit);

i2c_if_status_t i2c1_mem_write(uint8_t dev_addr_7bit, uint8_t reg, uint8_t *data, uint16_t len);
i2c_if_status_t i2c1_mem_read(uint8_t dev_addr_7bit, uint8_t reg, uint8_t *data, uint16_t len);


#endif /* I2C_DRIVER_H_ */
