#define STM32F405xx
#include "stm32f4xx.h"
#include "spi_if.h"
#include "tmc5160_driver.h"

///////////////////////////////////////////////////Definiciones///////////////////////////////////////////////////

#define TMC_WRITE_BIT             0x80u

#define TMC_GCONF                 0x00u
#define TMC_GLOBAL_SCALER         0x0Bu
#define TMC_IHOLD_IRUN            0x10u
#define TMC_TPOWERDOWN            0x11u
#define TMC_CHOPCONF              0x6Cu

////////////////////////////////////////////////Resoluciones MicroStep////////////////////////////////////////////

#define TMC_MICROSTEP_256         0x00000000u
#define TMC_MICROSTEP_128         0x00000001u
#define TMC_MICROSTEP_64          0x00000002u
#define TMC_MICROSTEP_32          0x00000003u
#define TMC_MICROSTEP_16          0x00000004u
#define TMC_MICROSTEP_8           0x00000005u
#define TMC_MICROSTEP_4           0x00000006u
#define TMC_MICROSTEP_2           0x00000007u
#define TMC_MICROSTEP_FULL        0x00000008u

////////////////////////////////////////////////Configuracion Motor///////////////////////////////////////////////

#define TMC_GLOBAL_SCALER_VALUE   110u
#define TMC_IHOLD_VALUE           6u
#define TMC_IRUN_VALUE            18u
#define TMC_IHOLDDELAY_VALUE      6u

/*
 * CHOPCONF:  16 microsteps
 */

#define TMC_CHOPCONF_VALUE        0x140100C3u

static uint32_t CHOPCONF_Configuracion = TMC_CHOPCONF_VALUE;

////////////////////////////////////////////////Funciones Privadas////////////////////////////////////////////////

static void Escritura_TMC5160(uint8_t Direccion, uint32_t Dato)
{
    uint8_t BufferTX[5];
    uint8_t BufferRX[5];

    BufferTX[0] = Direccion | TMC_WRITE_BIT;

    BufferTX[1] = (uint8_t)(Dato >> 24u);
    BufferTX[2] = (uint8_t)(Dato >> 16u);
    BufferTX[3] = (uint8_t)(Dato >> 8u);
    BufferTX[4] = (uint8_t)(Dato);

    spi1_STM32.transfer(BufferTX, BufferRX, 5u);
}

///////////////////////////////////////////////////Funciones//////////////////////////////////////////////////////

void tmc5160_pins_init(void)
{
    /////////////////////////////////////////////Clock GPIOB//////////////////////////////////////////////////////

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    //////////////////////////////////////////Configuracion PB6 PB7 PB8//////////////////////////////////////////

    GPIOB->MODER &= ~((3u << (6u * 2u)) |
                      (3u << (7u * 2u)) |
                      (3u << (8u * 2u)));

    GPIOB->MODER |=  ((1u << (6u * 2u)) |
                      (1u << (7u * 2u)) |
                      (1u << (8u * 2u)));

    //////////////////////////////////////////////Push Pull//////////////////////////////////////////////////////

    GPIOB->OTYPER &= ~((1u << 6u) |
                       (1u << 7u) |
                       (1u << 8u));

    /////////////////////////////////////////////Alta velocidad//////////////////////////////////////////////////

    GPIOB->OSPEEDR |= ((3u << (6u * 2u)) |
                       (3u << (7u * 2u)) |
                       (3u << (8u * 2u)));

    //////////////////////////////////////////Sin pull-up ni pull-down///////////////////////////////////////////

    GPIOB->PUPDR &= ~((3u << (6u * 2u)) |
                      (3u << (7u * 2u)) |
                      (3u << (8u * 2u)));

    tmc5160_disable();
}

void tmc5160_config(void)
{
    uint32_t ConfiguracionCorriente = 0;

    tmc5160_disable();

    //////////////////////////////////////////Configuracion GCONF////////////////////////////////////////////////

    Escritura_TMC5160(TMC_GCONF, 0x0000000Cu);

    /////////////////////////////////////////Escalador global de corriente///////////////////////////////////////

    Escritura_TMC5160(TMC_GLOBAL_SCALER, TMC_GLOBAL_SCALER_VALUE);

    //////////////////////////////////////////Configuracion corriente////////////////////////////////////////////

    ConfiguracionCorriente = ((uint32_t)TMC_IHOLDDELAY_VALUE << 16u) |
                             ((uint32_t)TMC_IRUN_VALUE << 8u) |
                             ((uint32_t)TMC_IHOLD_VALUE);

    Escritura_TMC5160(TMC_IHOLD_IRUN, ConfiguracionCorriente);

    //////////////////////////////////////////Tiempo power down//////////////////////////////////////////////////

    Escritura_TMC5160(TMC_TPOWERDOWN, 10u);

    //////////////////////////////////////////Configuracion CHOPCONF/////////////////////////////////////////////

    //Escritura_TMC5160(TMC_CHOPCONF, TMC_CHOPCONF_VALUE);

    Escritura_TMC5160(TMC_CHOPCONF, CHOPCONF_Configuracion);
}

void tmc5160_enable(void)
{
    GPIOB->BSRR = (uint32_t)(1u << (6u + 16u));        // Enable activo en bajo
}

void tmc5160_disable(void)
{
    GPIOB->BSRR = (uint32_t)(1u << 6u);                // Driver deshabilitado
}

void tmc5160_dir_clockwise(void)
{
    GPIOB->BSRR = (uint32_t)(1u << (8u + 16u));        // Giro horario
}

void tmc5160_dir_counterclockwise(void)
{
    GPIOB->BSRR = (uint32_t)(1u << 8u);                // Giro antihorario
}

void tmc5160_step_pulse(uint32_t delay_count)
{
    volatile uint32_t i = 0;

    //////////////////////////////////////////////Pulso STEP HIGH////////////////////////////////////////////////

    GPIOB->BSRR = (uint32_t)(1u << 7u);

    for(i = 0u ; i < delay_count ; i++)
    {
        __NOP();
    }

    //////////////////////////////////////////////Pulso STEP LOW/////////////////////////////////////////////////

    GPIOB->BSRR = (uint32_t)(1u << (7u + 16u));

    for(i = 0u ; i < delay_count ; i++)
    {
        __NOP();
    }
}

void tmc5160_set_microstep_resolution(uint16_t MicroSteps)
{
    uint32_t Resolucion = TMC_MICROSTEP_16;

    //////////////////////////////////////////Seleccion resolucion///////////////////////////////////////////////

    switch(MicroSteps)
    {
        case 256:

            Resolucion = TMC_MICROSTEP_256;

        break;

        case 128:

            Resolucion = TMC_MICROSTEP_128;

        break;

        case 64:

            Resolucion = TMC_MICROSTEP_64;

        break;

        case 32:

            Resolucion = TMC_MICROSTEP_32;

        break;

        case 16:

            Resolucion = TMC_MICROSTEP_16;

        break;

        case 8:

            Resolucion = TMC_MICROSTEP_8;

        break;

        case 4:

            Resolucion = TMC_MICROSTEP_4;

        break;

        case 2:

            Resolucion = TMC_MICROSTEP_2;

        break;

        default:

            Resolucion = TMC_MICROSTEP_16;

        break;
    }

    /*
     * Bits MRES[24:27]
     * Datasheet pagina 95
     */

    CHOPCONF_Configuracion &= ~(0x0Fu << 24u);

    CHOPCONF_Configuracion |= (Resolucion << 24u);

    Escritura_TMC5160(TMC_CHOPCONF, CHOPCONF_Configuracion);
}
