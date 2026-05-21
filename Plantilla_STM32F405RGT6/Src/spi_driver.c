#define STM32F405xx
#include "stm32f4xx.h"
#include "spi_driver.h"

#define SPI_TIMEOUT_COUNT 100000u
#define SPI_RESPONSE_DELAY_COUNT 20000u

/*
 * SPI1:
 * PB5 = CS manual, PA5 = SCK, PA6 = MISO, PA7 = MOSI.
 *
 * Datasheet DS8626:
 * - Pag. 51: PA5/PA6/PA7 tienen funcion SPI1_SCK/MISO/MOSI.
 * - Pag. 63: SPI1 usa AF5.
 *
 * Reference manual RM0090:
 * - Pag. 246: RCC_AHB1ENR GPIOAEN/GPIOBEN.
 * - Pags. 250-251: RCC_APB2ENR SPI1EN.
 * - Pags. 274 y 288: GPIO alternate function y AFRL.
 * - Pags. 919-923: SPI_CR1, SPI_SR, SPI_DR.
 */

static spi_if_status_t spi1_wait_flag_set(volatile uint32_t *reg, uint32_t flag)
{
    uint32_t timeout = SPI_TIMEOUT_COUNT;

    while (((*reg) & flag) == 0u) {
        if (timeout-- == 0u) {
            return SPI_IF_TIMEOUT;
        }
    }

    return SPI_IF_OK;
}

static spi_if_status_t spi1_wait_flag_clear(volatile uint32_t *reg, uint32_t flag)
{
    uint32_t timeout = SPI_TIMEOUT_COUNT;

    while (((*reg) & flag) != 0u) {
        if (timeout-- == 0u) {
            return SPI_IF_TIMEOUT;
        }
    }

    return SPI_IF_OK;
}

void spi1_cs_low(void)
{
    GPIOB->BSRR = (uint32_t)(1u << (5u + 16u));
}

void spi1_cs_high(void)
{
    GPIOB->BSRR = (uint32_t)(1u << 5u);
}

static void spi1_response_delay(void)
{
    volatile uint32_t count = SPI_RESPONSE_DELAY_COUNT;

    while (count-- != 0u) {
        __NOP();
    }
}

void spi1_pa5_pa6_pa7_init_mode3_prescaler32(void)
{
    /* RM0090 pag. 246: habilitar clock GPIOA para SPI1 y GPIOB para CS manual. */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;

    /* RM0090 pags. 250-251: habilitar clock SPI1 en APB2. */
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    /* PB5 como CS manual en salida push-pull, activo en bajo. */
    GPIOB->MODER &= ~(3u << (5u * 2u));
    GPIOB->MODER |=  (1u << (5u * 2u));
    GPIOB->OTYPER &= ~(1u << 5u);
    GPIOB->OSPEEDR |= (3u << (5u * 2u));
    GPIOB->PUPDR &= ~(3u << (5u * 2u));
    spi1_cs_high();

    /* PA5, PA6, PA7 como alternate function. RM0090 pag. 274. */
    GPIOA->MODER &= ~((3u << (5u * 2u)) | (3u << (6u * 2u)) | (3u << (7u * 2u)));
    GPIOA->MODER |=  ((2u << (5u * 2u)) | (2u << (6u * 2u)) | (2u << (7u * 2u)));

    /* Push-pull, alta velocidad, sin pull. */
    GPIOA->OTYPER &= ~((1u << 5u) | (1u << 6u) | (1u << 7u));
    GPIOA->OSPEEDR |= ((3u << (5u * 2u)) | (3u << (6u * 2u)) | (3u << (7u * 2u)));
    GPIOA->PUPDR &= ~((3u << (5u * 2u)) | (3u << (6u * 2u)) | (3u << (7u * 2u)));

    /* AF5 para SPI1 en PA5/PA6/PA7. DS8626 pag. 63, RM0090 pag. 288. */
    GPIOA->AFR[0] &= ~((0xFu << (5u * 4u)) | (0xFu << (6u * 4u)) | (0xFu << (7u * 4u)));
    GPIOA->AFR[0] |=  ((5u   << (5u * 4u)) | (5u   << (6u * 4u)) | (5u   << (7u * 4u)));

    /* Deshabilitar SPI antes de configurar. RM0090 pag. 920: SPE. */
    SPI1->CR1 &= ~SPI_CR1_SPE;

    /*
     * Configuracion:
     * - Master: MSTR=1. RM0090 pag. 920.
     * - Full-duplex 2 lineas: BIDIMODE=0, RXONLY=0. RM0090 pags. 919-920.
     * - 8 bits: DFF=0. RM0090 pag. 919.
     * - NSS por software: SSM=1, SSI=1. RM0090 pag. 920.
     * - MSB first: LSBFIRST=0. RM0090 pag. 920.
     * - Baud prescaler fPCLK/256: BR=111. Con APB2=84 MHz, SCK aprox 328 kHz.
     *   Esta velocidad da margen al Arduino UNO para cargar SPDR desde la ISR.
     * - Mode 3: CPOL=1, CPHA=1. RM0090 pag. 921.
     */
    SPI1->CR1 = SPI_CR1_MSTR
              | SPI_CR1_SSM
              | SPI_CR1_SSI
              | SPI_CR1_BR_2
              | SPI_CR1_BR_1
              | SPI_CR1_BR_0
              | SPI_CR1_CPOL
              | SPI_CR1_CPHA;

    SPI1->CR2 = 0u;

    /* Habilitar SPI. */
    SPI1->CR1 |= SPI_CR1_SPE;
}

spi_if_status_t spi1_transfer_byte(uint8_t tx, uint8_t *rx)
{
    uint8_t received;

    if (rx == 0) {
        return SPI_IF_ERROR;
    }

    /* RM0090 pags. 889-890: esperar TXE, escribir DR, esperar RXNE, leer DR. */
    if (spi1_wait_flag_set(&SPI1->SR, SPI_SR_TXE) != SPI_IF_OK) {
        return SPI_IF_TIMEOUT;
    }

    *(__IO uint8_t *)&SPI1->DR = tx;

    if (spi1_wait_flag_set(&SPI1->SR, SPI_SR_RXNE) != SPI_IF_OK) {
        return SPI_IF_TIMEOUT;
    }

    received = *(__IO uint8_t *)&SPI1->DR;
    *rx = received;

    if (spi1_wait_flag_set(&SPI1->SR, SPI_SR_TXE) != SPI_IF_OK) {
        return SPI_IF_TIMEOUT;
    }

    if (spi1_wait_flag_clear(&SPI1->SR, SPI_SR_BSY) != SPI_IF_OK) {
        return SPI_IF_TIMEOUT;
    }

    return SPI_IF_OK;
}

spi_if_status_t spi1_transfer(uint8_t *tx, uint8_t *rx, uint16_t len)
{
    uint16_t i;
    uint8_t tx_byte;
    uint8_t rx_byte;

    if ((rx == 0) || (len == 0u)) {
        return SPI_IF_ERROR;
    }

    spi1_cs_low();

    for (i = 0; i < len; i++) {
        tx_byte = (tx != 0) ? tx[i] : 0xFFu;

        if (spi1_transfer_byte(tx_byte, &rx_byte) != SPI_IF_OK) {
            spi1_cs_high();
            return SPI_IF_TIMEOUT;
        }

        rx[i] = rx_byte;
    }

    spi1_cs_high();

    return SPI_IF_OK;
}

spi_if_status_t spi1_send_then_read_byte(uint8_t tx, uint8_t dummy, uint8_t *response)
{
    uint8_t discard;

    if (response == 0) {
        return SPI_IF_ERROR;
    }

    /*
     * SPI es full-duplex: siempre se recibe algo mientras se transmite.
     * Si el esclavo responde en el siguiente frame, se manda un dummy para generar clock.
     * RM0090 pags. 888-890.
     */
    spi1_cs_low();

    if (spi1_transfer_byte(tx, &discard) != SPI_IF_OK) {
        spi1_cs_high();
        return SPI_IF_TIMEOUT;
    }

    spi1_response_delay();

    if (spi1_transfer_byte(dummy, response) != SPI_IF_OK) {
        spi1_cs_high();
        return SPI_IF_TIMEOUT;
    }

    spi1_cs_high();

    return SPI_IF_OK;
}
