#define STM32F405xx
#include "stm32f4xx.h"
#include "system_clock_if.h"
#include "uart_if.h"
#include "spi_if.h"
#include "tmc5160_driver.h"

///////////////////////////////////////////////////Definiciones///////////////////////////////////////////////////

#define LIMIT_GPIO                GPIOB
#define LIMIT_RCC_EN              RCC_AHB1ENR_GPIOBEN
#define LIMIT_PIN                 2u

///////////////////////////////////////////Variables Globales/////////////////////////////////////////////////////

static uint8_t MotorActivo = 0u;
static uint8_t RegresandoHome = 0u;

static volatile int32_t PasosDesdeHome = 0;

static volatile uint32_t VueltasMotor = 0u;

static uint32_t DelayVelocidadMotor = 30000u;

static uint16_t ResolucionMicroStep = 16u;

static uint32_t PasosPorVuelta = 3200u;

////////////////////////////////////////////////Funciones Privadas////////////////////////////////////////////////

static void Configuracion_Limit_Switch(void);

static uint8_t UART_InitMotor_Recibido(void);

static uint8_t Limit_Switch_Activado(void);

static void Configuracion_Velocidad_Motor(uint32_t Velocidad);

static void Configuracion_MicroStep(uint16_t MicroStep);

////////////////////////////////////////////////////Funciones/////////////////////////////////////////////////////

static void Configuracion_Velocidad_Motor(uint32_t Velocidad)
{
    DelayVelocidadMotor = Velocidad;
}

static void Configuracion_MicroStep(uint16_t MicroStep)
{
    /*
     * Valores:
     *
     * 256
     * 128
     * 64
     * 32
     * 16
     * 8
     * 4
     * 2
     */

    ResolucionMicroStep = MicroStep;

    /*
     * NEMA17:
     * 200 pasos por vuelta
     */

    PasosPorVuelta = 200u * ResolucionMicroStep;

    tmc5160_set_microstep_resolution(ResolucionMicroStep);
}

static uint8_t UART_InitMotor_Recibido(void)
{
    static const char Comando[] = "INITMOTOR";

    static uint8_t Indice = 0u;

    uint8_t ByteRecibido = 0u;

    while(usart_STM32.read_byte_nonblocking(&ByteRecibido) != 0)
    {
        if(ByteRecibido == (uint8_t)Comando[Indice])
        {
            Indice++;

            if(Comando[Indice] == '\0')
            {
                Indice = 0u;

                return 1u;
            }
        }
        else
        {
            Indice = (ByteRecibido == (uint8_t)Comando[0]) ? 1u : 0u;
        }
    }

    return 0u;
}

static void Configuracion_Limit_Switch(void)
{
    RCC->AHB1ENR |= LIMIT_RCC_EN;

    LIMIT_GPIO->MODER &= ~(3u << (LIMIT_PIN * 2u));

    LIMIT_GPIO->PUPDR &= ~(3u << (LIMIT_PIN * 2u));

    LIMIT_GPIO->PUPDR |=  (2u << (LIMIT_PIN * 2u));
}

static uint8_t Limit_Switch_Activado(void)
{
    return ((LIMIT_GPIO->IDR & (1u << LIMIT_PIN)) != 0u) ? 1u : 0u;
}

/////////////////////////////////////////////////////Main/////////////////////////////////////////////////////////

int main(void)
{
    //////////////////////////////////////////Inicializacion Sistema//////////////////////////////////////////////

    SYSCLK_STM32.init();

    usart_STM32.init_115200();

    spi1_STM32.init();

    //////////////////////////////////////////Inicializacion Hardware/////////////////////////////////////////////

    Configuracion_Limit_Switch();

    tmc5160_pins_init();

    tmc5160_config();

    /////////////////////////////////////////Configuraciones Iniciales////////////////////////////////////////////

    Configuracion_MicroStep(8);

    /*
     * CONFIGURACION DE MICROSTEPS
     *
     * Valores permitidos:
     *
     * 256
     * 128
     * 64
     * 32
     * 16
     * 8
     * 4
     * 2
     * Full Steep
     * FORMULA:
     *
     * PasosPorVuelta = 200 * MicroStep
     *
     * EJEMPLOS:
     *
     * 200 * 2   = 400 pasos/vuelta
     * 200 * 4   = 800 pasos/vuelta
     * 200 * 8   = 1600 pasos/vuelta
     * 200 * 16  = 3200 pasos/vuelta
     * 200 * 32  = 6400 pasos/vuelta
     * 200 * 64  = 12800 pasos/vuelta
     * 200 * 128 = 25600 pasos/vuelta
     * 200 * 256 = 51200 pasos/vuelta
     *
     * RECOMENDACION:
     *
     * 16 microsteps:
     * - Muy estable
     * - Buen torque
     * - Movimiento suave
     * - Recomendado para uso general
     *
     * 32 o 64 microsteps:
     * - Movimiento mas suave
     * - Menor torque a altas velocidades
     *
     * 2, 4 u 8 microsteps:
     * - Mayor torque
     * - Menor suavidad
     * - Mayor velocidad maxima
     *
     */

    Configuracion_Velocidad_Motor(2500);

    /*
     * CONFIGURACION DE VELOCIDAD DEL MOTOR
     *
     * IMPORTANTE:
     * Entre menor sea el valor, Mayor velocidad del motor
     *
     *
     * Entre mayor sea el valor, Menor velocidad del motor
     *
     *
     * Valores recomendados:
     *
     * 5000     -> Muy rapido
     * 8000     -> Rapido
     * 15000    -> Velocidad media alta
     * 30000    -> Velocidad estable RECOMENDADA
     * 50000    -> Lento
     * 80000    -> Muy lento
     *
     * NOTA:
     * Si el valor es demasiado pequeno:
     * - El motor puede perder pasos
     * - Puede vibrar
     * - Puede no arrancar
     *
     * Para motores NEMA17 a 14.8V:
     * Un rango seguro es:
     *
     * 8000 -> 50000
     *
     */

    //////////////////////////////////////////////Mensaje UART////////////////////////////////////////////////////

    usart_STM32.write_str("TMC5160 NEMA17 LISTO\r\n");

    //////////////////////////////////////////////Loop principal/////////////////////////////////////////////////

    while(1)
    {
        //////////////////////////////////////////Comando INITMOTOR///////////////////////////////////////////////

        if(UART_InitMotor_Recibido() != 0u)
        {
            PasosDesdeHome = 0;

            VueltasMotor = 0;

            MotorActivo = 1u;

            RegresandoHome = 0u;

            tmc5160_enable();

            tmc5160_dir_clockwise();

            usart_STM32.write_str("MOTOR INICIA\r\n");
        }

        ///////////////////////////////////////////Deteccion Limit Switch////////////////////////////////////////

        else if((MotorActivo == 1u) && (RegresandoHome == 0u) && (Limit_Switch_Activado() == 1u))
        {
            RegresandoHome = 1u;

            tmc5160_dir_counterclockwise();

            usart_STM32.write_str("LIMIT SWITCH DETECTADO\r\n");
        }

        //////////////////////////////////////////////Movimiento horario/////////////////////////////////////////

        else if((MotorActivo == 1u) && (RegresandoHome == 0u))
        {
            tmc5160_step_pulse(DelayVelocidadMotor);

            PasosDesdeHome++;

            /////////////////////////////////////////Conteo de vueltas////////////////////////////////////////////

            if((PasosDesdeHome % PasosPorVuelta) == 0u)
            {
                VueltasMotor++;
            }
        }

        /////////////////////////////////////////////Regreso a HOME///////////////////////////////////////////////

        else if((MotorActivo == 1u) && (RegresandoHome == 1u))
        {
            if(PasosDesdeHome > 0)
            {
            	tmc5160_step_pulse(DelayVelocidadMotor);

            	PasosDesdeHome--;

            	/////////////////////////////////////////Descuento de vueltas///////////////////////////////////////////////

            	if((PasosDesdeHome % PasosPorVuelta) == 0u)   //Lo que hace es dividir los pulsos entre el valor entero de pasos por vuelta (3200)
            	{										      // y cuando no hay residuo osea que la divison dio un entero entre y descuenta 1 vuelta
            	    if(VueltasMotor > 0u)				      // pregunta si las vueltas de motor son mayor a cero la descuenta para no dar numero negativos.
            	    {
            	         VueltasMotor--;
            	     }
            	}
            }
            else
            {
                tmc5160_disable();

                MotorActivo = 0u;

                RegresandoHome = 0u;

                usart_STM32.write_str("MOTOR EN HOME\r\n");
            }
        }

        //////////////////////////////////////////////Esperando comando///////////////////////////////////////////

        else
        {

        }
    }
}
