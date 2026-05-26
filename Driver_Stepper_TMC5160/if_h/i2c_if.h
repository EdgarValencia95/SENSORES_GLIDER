#ifndef I2C_IF_H_
#define I2C_IF_H_

#include <stdint.h>

typedef enum {
    I2C_IF_OK = 0,
    I2C_IF_ERROR = 1,
    I2C_IF_TIMEOUT = 2
} i2c_if_status_t;

typedef struct {
    void (*init_100khz)(void);
    i2c_if_status_t (*mem_write)(uint8_t dev_addr_7bit, uint8_t reg, uint8_t *data, uint16_t len);
    i2c_if_status_t (*mem_read)(uint8_t dev_addr_7bit, uint8_t reg, uint8_t *data, uint16_t len);
} i2c_if_t;

extern const i2c_if_t i2c1_STM32;

#endif /* I2C_IF_H_ */
