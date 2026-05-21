#define STM32F405xx
#include "stm32f4xx.h"
#include "uart_driver.h"
#define UART_BAUD   115200u

static uint32_t usart_brr_from_pclk(uint32_t pclk_hz, uint32_t baud)
{
    // USART oversampling by 16 (OVER8=0)
    // BRR = mantissa<<4 | fraction, where fraction = round(16 * (pclk/baud - mantissa))
    // Usamos redondeo entero: (pclk + baud/2) / baud  -> da USARTDIV*16
    uint32_t usartdiv_x16 = (pclk_hz + (baud/2u)) / baud;
    return usartdiv_x16; // Para OVER16, BRR codifica directamente USARTDIV*16
}

void usart3_pb10_pb11_init_115200(void)
{
    // 1) Clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;     // GPIOB clock
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;    // USART3 clock (APB1)

    // 2) GPIOB: PB10=TX, PB11=RX -> Alternate Function AF7
    // MODER: 10b = AF
    GPIOB->MODER &= ~((3u << (10*2)) | (3u << (11*2)));
    GPIOB->MODER |=  ((2u << (10*2)) | (2u << (11*2)));

    // OTYPER: push-pull (0)
    GPIOB->OTYPER &= ~((1u << 10) | (1u << 11));

    // OSPEEDR: high speed (opcional)
    GPIOB->OSPEEDR |= ((3u << (10*2)) | (3u << (11*2)));

    // PUPDR: RX con pull-up (recomendado), TX sin pull
    GPIOB->PUPDR &= ~((3u << (10*2)) | (3u << (11*2)));
    GPIOB->PUPDR |=  (1u << (11*2)); // PB11 pull-up

    // AFRH (pins 8..15): AF7 = 0b0111
    GPIOB->AFR[1] &= ~((0xFu << ((10-8)*4)) | (0xFu << ((11-8)*4)));
    GPIOB->AFR[1] |=  ((7u   << ((10-8)*4)) | (7u   << ((11-8)*4)));

    // 3) USART3 config: 8N1, oversampling 16
    USART3->CR1 = 0;                 // limpia config
    USART3->CR2 = 0;
    USART3->CR3 = 0;

    // 4) Baud rate usando PCLK1 real
    // Si NO tienes una función para leer clocks, asumo PCLK1 = 42 MHz (típico con HCLK=168MHz, APB1 presc=4).
    // Cambia PCLK1_HZ si tu clock tree es distinto.
    const uint32_t PCLK1_HZ = 42000000u;
    USART3->BRR = usart_brr_from_pclk(PCLK1_HZ, UART_BAUD);

    // 5) Enable TX, RX, USART
    USART3->CR1 |= USART_CR1_TE | USART_CR1_RE;
    USART3->CR1 |= USART_CR1_UE;

}

void usart3_write_byte(uint8_t b)
{
    USART3->DR = b;
    while (!(USART3->SR & USART_SR_TXE)) {}     // TX buffer empty
}

void usart3_write_str(const char *s)
{
    while (*s) 
    {
        usart3_write_byte((uint8_t)*s++);
    }
}

int usart3_read_byte_nonblocking(uint8_t *out)
{
    if (USART3->SR & USART_SR_RXNE) 
    {
        *out = (uint8_t)(USART3->DR & 0xFFu);
        return 1;
    }
    return 0;
}

uint8_t usart3_read_byte_blocking(void)
{
    while (!(USART3->SR & USART_SR_RXNE)) {}
    return (uint8_t)(USART3->DR & 0xFFu);
}
void uart3_flush_tc(void)
{
    while(!(USART3->SR & USART_SR_TC)) {}
}
