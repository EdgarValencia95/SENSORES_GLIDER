#define STM32F405xx
#include "stm32f4xx.h"
#include "i2c_driver.h"

#define I2C_TIMEOUT_COUNT  100000u

static i2c_if_status_t i2c1_wait_flag_set(volatile uint32_t *reg, uint32_t flag)
{
    uint32_t timeout = I2C_TIMEOUT_COUNT;

    while (((*reg) & flag) == 0u) {
        if (timeout-- == 0u) {
            return I2C_IF_TIMEOUT;
        }
    }

    return I2C_IF_OK;
}

static i2c_if_status_t i2c1_wait_flag_clear(volatile uint32_t *reg, uint32_t flag)
{
    uint32_t timeout = I2C_TIMEOUT_COUNT;

    while (((*reg) & flag) != 0u) {
        if (timeout-- == 0u) {
            return I2C_IF_TIMEOUT;
        }
    }

    return I2C_IF_OK;
}

static void i2c1_clear_addr_flag(void)
{
    volatile uint32_t tmp;

    tmp = I2C1->SR1;
    tmp = I2C1->SR2;
    (void)tmp;
}

void i2c1_pb6_pb7_init_100khz(void)
{
    /* GPIOB clock + I2C1 clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    /* PB6=SCL, PB7=SDA -> Alternate Function */
    GPIOB->MODER &= ~((3u << (6u * 2u)) | (3u << (7u * 2u)));
    GPIOB->MODER |=  ((2u << (6u * 2u)) | (2u << (7u * 2u)));

    /* Open-drain */
    GPIOB->OTYPER |= (1u << 6u) | (1u << 7u);

    /* High speed */
    GPIOB->OSPEEDR |= (3u << (6u * 2u)) | (3u << (7u * 2u));

    /* Pull-up interno. Aun asi se recomiendan pull-ups externos en el bus. */
    GPIOB->PUPDR &= ~((3u << (6u * 2u)) | (3u << (7u * 2u)));
    GPIOB->PUPDR |=  ((1u << (6u * 2u)) | (1u << (7u * 2u)));

    /* AF4 para I2C1 en PB6/PB7 */
    GPIOB->AFR[0] &= ~((0xFu << (6u * 4u)) | (0xFu << (7u * 4u)));
    GPIOB->AFR[0] |=  ((4u   << (6u * 4u)) | (4u   << (7u * 4u)));

    /* Reset de I2C1 */
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;

    /* Software reset del periferico I2C */
    I2C1->CR1 |= I2C_CR1_SWRST;
    I2C1->CR1 &= ~I2C_CR1_SWRST;

    /* Deshabilitar periferico antes de configurar */
    I2C1->CR1 &= ~I2C_CR1_PE;

    /*
     * PCLK1 = 42 MHz, segun tu SystemClock_Config:
     * SYSCLK=168 MHz, APB1=42 MHz.
     *
     * I2C Standard Mode 100 kHz:
     * CR2   = 42 MHz
     * CCR   = PCLK1 / (2 * I2C_FREQ) = 42000000 / 200000 = 210
     * TRISE = FREQ + 1 = 43
     */
    I2C1->CR2 = 42u;
    I2C1->CCR = 210u;
    I2C1->TRISE = 43u;

    /* ACK habilitado por defecto */
    I2C1->CR1 |= I2C_CR1_ACK;

    /* Habilitar I2C1 */
    I2C1->CR1 |= I2C_CR1_PE;
}

i2c_if_status_t i2c1_mem_write(uint8_t dev_addr_7bit, uint8_t reg, uint8_t *data, uint16_t len)
{
    uint16_t i;

    if (i2c1_wait_flag_clear(&I2C1->SR2, I2C_SR2_BUSY) != I2C_IF_OK) {
        return I2C_IF_TIMEOUT;
    }

    I2C1->CR1 |= I2C_CR1_START;

    if (i2c1_wait_flag_set(&I2C1->SR1, I2C_SR1_SB) != I2C_IF_OK) {
        return I2C_IF_TIMEOUT;
    }

    I2C1->DR = (uint8_t)(dev_addr_7bit << 1);

    if (i2c1_wait_flag_set(&I2C1->SR1, I2C_SR1_ADDR) != I2C_IF_OK) {
        return I2C_IF_TIMEOUT;
    }

    i2c1_clear_addr_flag();

    if (i2c1_wait_flag_set(&I2C1->SR1, I2C_SR1_TXE) != I2C_IF_OK) {
        return I2C_IF_TIMEOUT;
    }

    I2C1->DR = reg;

    for (i = 0; i < len; i++) {
        if (i2c1_wait_flag_set(&I2C1->SR1, I2C_SR1_TXE) != I2C_IF_OK) {
            return I2C_IF_TIMEOUT;
        }

        I2C1->DR = data[i];
    }

    if (i2c1_wait_flag_set(&I2C1->SR1, I2C_SR1_BTF) != I2C_IF_OK) {
        return I2C_IF_TIMEOUT;
    }

    I2C1->CR1 |= I2C_CR1_STOP;

    return I2C_IF_OK;
}

i2c_if_status_t i2c1_mem_read(uint8_t dev_addr_7bit, uint8_t reg, uint8_t *data, uint16_t len)
{
    uint16_t i;

    if ((data == 0) || (len == 0u)) {
        return I2C_IF_ERROR;
    }

    if (i2c1_wait_flag_clear(&I2C1->SR2, I2C_SR2_BUSY) != I2C_IF_OK) {
        return I2C_IF_TIMEOUT;
    }

    I2C1->CR1 |= I2C_CR1_ACK;

    /* START + direccion write */
    I2C1->CR1 |= I2C_CR1_START;

    if (i2c1_wait_flag_set(&I2C1->SR1, I2C_SR1_SB) != I2C_IF_OK) {
        return I2C_IF_TIMEOUT;
    }

    I2C1->DR = (uint8_t)(dev_addr_7bit << 1);

    if (i2c1_wait_flag_set(&I2C1->SR1, I2C_SR1_ADDR) != I2C_IF_OK) {
        return I2C_IF_TIMEOUT;
    }

    i2c1_clear_addr_flag();

    if (i2c1_wait_flag_set(&I2C1->SR1, I2C_SR1_TXE) != I2C_IF_OK) {
        return I2C_IF_TIMEOUT;
    }

    I2C1->DR = reg;

    if (i2c1_wait_flag_set(&I2C1->SR1, I2C_SR1_BTF) != I2C_IF_OK) {
        return I2C_IF_TIMEOUT;
    }

    /* Repeated START + direccion read */
    I2C1->CR1 |= I2C_CR1_START;

    if (i2c1_wait_flag_set(&I2C1->SR1, I2C_SR1_SB) != I2C_IF_OK) {
        return I2C_IF_TIMEOUT;
    }

    I2C1->DR = (uint8_t)((dev_addr_7bit << 1) | 1u);

    if (i2c1_wait_flag_set(&I2C1->SR1, I2C_SR1_ADDR) != I2C_IF_OK) {
        return I2C_IF_TIMEOUT;
    }

    if (len == 1u) {
        I2C1->CR1 &= ~I2C_CR1_ACK;
        i2c1_clear_addr_flag();
        I2C1->CR1 |= I2C_CR1_STOP;

        if (i2c1_wait_flag_set(&I2C1->SR1, I2C_SR1_RXNE) != I2C_IF_OK) {
            return I2C_IF_TIMEOUT;
        }

        data[0] = (uint8_t)I2C1->DR;
    } else {
        i2c1_clear_addr_flag();

        for (i = 0; i < len; i++) {
            if (i == (len - 1u)) {
                I2C1->CR1 &= ~I2C_CR1_ACK;
                I2C1->CR1 |= I2C_CR1_STOP;
            }

            if (i2c1_wait_flag_set(&I2C1->SR1, I2C_SR1_RXNE) != I2C_IF_OK) {
                return I2C_IF_TIMEOUT;
            }

            data[i] = (uint8_t)I2C1->DR;
        }
    }

    I2C1->CR1 |= I2C_CR1_ACK;

    return I2C_IF_OK;
}

i2c_if_status_t i2c1_check_device(uint8_t dev_addr_7bit)
{
    uint32_t timeout;

    if (i2c1_wait_flag_clear(&I2C1->SR2, I2C_SR2_BUSY) != I2C_IF_OK) {
        return I2C_IF_TIMEOUT;
    }

    I2C1->CR1 |= I2C_CR1_START;

    if (i2c1_wait_flag_set(&I2C1->SR1, I2C_SR1_SB) != I2C_IF_OK) {
        return I2C_IF_TIMEOUT;
    }

    I2C1->DR = (uint8_t)(dev_addr_7bit << 1);

    timeout = I2C_TIMEOUT_COUNT;

    while ((I2C1->SR1 & I2C_SR1_ADDR) == 0u) {
        if (I2C1->SR1 & I2C_SR1_AF) {
            I2C1->SR1 &= ~I2C_SR1_AF;
            I2C1->CR1 |= I2C_CR1_STOP;
            return I2C_IF_ERROR;
        }

        if (timeout-- == 0u) {
            I2C1->CR1 |= I2C_CR1_STOP;
            return I2C_IF_TIMEOUT;
        }
    }

    i2c1_clear_addr_flag();
    I2C1->CR1 |= I2C_CR1_STOP;

    return I2C_IF_OK;
}




