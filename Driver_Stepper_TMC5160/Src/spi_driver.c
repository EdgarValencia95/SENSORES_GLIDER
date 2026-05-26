#define STM32F405xx
#include "stm32f4xx.h"
#include "spi_driver.h"

///////////////////////////////////////////////////Definiciones///////////////////////////////////////////////////

#define SPI_TIMEOUT_COUNT            100000u
#define SPI_RESPONSE_DELAY_COUNT     20000u

/*
 * SPI1:
 * PB5 = CS
 * PA5 = SCK
 * PA6 = MISO
 * PA7 = MOSI
 *
 * Datasheet STM32F405:
 * - PA5/PA6/PA7 trabajan con SPI1 utilizando AF5
 * - SPI1 trabaja sobre APB2
 */

/////////////////////////////////////////////////Funciones Privadas///////////////////////////////////////////////

static spi_if_status_t Espera_Bit_Activo(volatile uint32_t *Registro, uint32_t Bandera)
{
    uint32_t TiempoEspera = SPI_TIMEOUT_COUNT;

    while(((*Registro) & Bandera) == 0u)
    {
        if(TiempoEspera-- == 0u)
        {
            return SPI_IF_TIMEOUT;
        }
    }

    return SPI_IF_OK;
}

static spi_if_status_t Espera_Bit_Inactivo(volatile uint32_t *Registro, uint32_t Bandera)
{
    uint32_t TiempoEspera = SPI_TIMEOUT_COUNT;

    while(((*Registro) & Bandera) != 0u)
    {
        if(TiempoEspera-- == 0u)
        {
            return SPI_IF_TIMEOUT;
        }
    }

    return SPI_IF_OK;
}

static void Delay_Respuesta_SPI(void)
{
    volatile uint32_t Conteo = SPI_RESPONSE_DELAY_COUNT;

    while(Conteo-- != 0u)
    {
        __NOP();
    }
}

///////////////////////////////////////////////////Funciones SPI//////////////////////////////////////////////////

void spi1_cs_low(void)
{
    GPIOB->BSRR = (uint32_t)(1u << (5u + 16u));        // CS activo en bajo
}

void spi1_cs_high(void)
{
    GPIOB->BSRR = (uint32_t)(1u << 5u);                // CS desactivado
}

void spi1_pa5_pa6_pa7_init_mode3_prescaler32(void)
{
    /////////////////////////////////////////Habilitacion de perifericos/////////////////////////////////////////

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;               // Habilita clock del puerto A
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;               // Habilita clock del puerto B
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;                // Habilita clock de SPI1

    ///////////////////////////////////////////////Configuracion CS///////////////////////////////////////////////

    GPIOB->MODER &= ~(3u << (5u * 2u));                // Limpia configuracion PB5
    GPIOB->MODER |=  (1u << (5u * 2u));                // PB5 como salida

    GPIOB->OTYPER &= ~(1u << 5u);                      // Push Pull

    GPIOB->OSPEEDR |= (3u << (5u * 2u));               // Alta velocidad

    GPIOB->PUPDR &= ~(3u << (5u * 2u));                // Sin pull-up ni pull-down

    spi1_cs_high();                                    // CS deshabilitado inicialmente

    //////////////////////////////////////////Configuracion SPI1 GPIO////////////////////////////////////////////

    GPIOA->MODER &= ~((3u << (5u * 2u)) |
                      (3u << (6u * 2u)) |
                      (3u << (7u * 2u)));

    GPIOA->MODER |=  ((2u << (5u * 2u)) |
                      (2u << (6u * 2u)) |
                      (2u << (7u * 2u)));              // Alternate Function

    GPIOA->OTYPER &= ~((1u << 5u) |
                       (1u << 6u) |
                       (1u << 7u));                    // Push Pull

    GPIOA->OSPEEDR |= ((3u << (5u * 2u)) |
                       (3u << (6u * 2u)) |
                       (3u << (7u * 2u)));             // Alta velocidad

    GPIOA->PUPDR &= ~((3u << (5u * 2u)) |
                      (3u << (6u * 2u)) |
                      (3u << (7u * 2u)));              // Sin resistencias

    ///////////////////////////////////////////Alternate Function AF5////////////////////////////////////////////

    GPIOA->AFR[0] &= ~((0xFu << (5u * 4u)) |
                       (0xFu << (6u * 4u)) |
                       (0xFu << (7u * 4u)));

    GPIOA->AFR[0] |=  ((5u << (5u * 4u)) |
                       (5u << (6u * 4u)) |
                       (5u << (7u * 4u)));             // AF5 = SPI1

    //////////////////////////////////////////////Configuracion SPI//////////////////////////////////////////////

    SPI1->CR1 &= ~SPI_CR1_SPE;                         // Deshabilita SPI antes de configurar

    SPI1->CR1 = SPI_CR1_MSTR |                         // Master mode
                SPI_CR1_SSM   |                        // NSS software
                SPI_CR1_SSI   |                        // NSS interno alto
                SPI_CR1_BR_2  |                        // Baud rate prescaler
                SPI_CR1_BR_1  |
                SPI_CR1_BR_0  |
                SPI_CR1_CPOL  |                        // Clock polarity alto
                SPI_CR1_CPHA;                          // Clock phase segundo flanco

    SPI1->CR2 = 0x00000000;                            // SPI sin interrupciones

    SPI1->CR1 |= SPI_CR1_SPE;                          // Habilita SPI1
}

spi_if_status_t spi1_transfer_byte(uint8_t tx, uint8_t *rx)
{
    uint8_t DatoRecibido = 0;

    if(rx == 0)
    {
        return SPI_IF_ERROR;
    }

    //////////////////////////////////////////////Espera TXE/////////////////////////////////////////////////////

    if(Espera_Bit_Activo(&SPI1->SR, SPI_SR_TXE) != SPI_IF_OK)
    {
        return SPI_IF_TIMEOUT;
    }

    /////////////////////////////////////////////Envio de dato///////////////////////////////////////////////////

    *(__IO uint8_t *)&SPI1->DR = tx;

    /////////////////////////////////////////////Espera RXNE/////////////////////////////////////////////////////

    if(Espera_Bit_Activo(&SPI1->SR, SPI_SR_RXNE) != SPI_IF_OK)
    {
        return SPI_IF_TIMEOUT;
    }

    /////////////////////////////////////////////Lectura SPI/////////////////////////////////////////////////////

    DatoRecibido = *(__IO uint8_t *)&SPI1->DR;

    *rx = DatoRecibido;

    //////////////////////////////////////////////Espera TXE/////////////////////////////////////////////////////

    if(Espera_Bit_Activo(&SPI1->SR, SPI_SR_TXE) != SPI_IF_OK)
    {
        return SPI_IF_TIMEOUT;
    }

    //////////////////////////////////////////////Espera BSY/////////////////////////////////////////////////////

    if(Espera_Bit_Inactivo(&SPI1->SR, SPI_SR_BSY) != SPI_IF_OK)
    {
        return SPI_IF_TIMEOUT;
    }

    return SPI_IF_OK;
}

spi_if_status_t spi1_transfer(uint8_t *tx, uint8_t *rx, uint16_t len)
{
    uint16_t i = 0;
    uint8_t ByteTX = 0;
    uint8_t ByteRX = 0;

    if((rx == 0) || (len == 0u))
    {
        return SPI_IF_ERROR;
    }

    spi1_cs_low();

    for(i = 0 ; i < len ; i++)
    {
        ByteTX = (tx != 0) ? tx[i] : 0xFFu;

        if(spi1_transfer_byte(ByteTX, &ByteRX) != SPI_IF_OK)
        {
            spi1_cs_high();

            return SPI_IF_TIMEOUT;
        }

        rx[i] = ByteRX;
    }

    spi1_cs_high();

    return SPI_IF_OK;
}

spi_if_status_t spi1_send_then_read_byte(uint8_t tx, uint8_t dummy, uint8_t *response)
{
    uint8_t DatoDummy = 0;

    if(response == 0)
    {
        return SPI_IF_ERROR;
    }

    spi1_cs_low();

    /////////////////////////////////////////////Primer envio////////////////////////////////////////////////////

    if(spi1_transfer_byte(tx, &DatoDummy) != SPI_IF_OK)
    {
        spi1_cs_high();

        return SPI_IF_TIMEOUT;
    }

    /////////////////////////////////////////////Delay respuesta/////////////////////////////////////////////////

    Delay_Respuesta_SPI();

    //////////////////////////////////////////////Lectura SPI////////////////////////////////////////////////////

    if(spi1_transfer_byte(dummy, response) != SPI_IF_OK)
    {
        spi1_cs_high();

        return SPI_IF_TIMEOUT;
    }

    spi1_cs_high();

    return SPI_IF_OK;
}
