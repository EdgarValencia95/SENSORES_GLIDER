#define STM32F405xx
#include "stm32f4xx.h"
#include "system_clock_if.h"
#include "uart_if.h"
#include "spi_if.h"

#define SPI_CMD_TEST        0x15u
#define SPI_DUMMY_BYTE      0x00u
#define SPI_EXPECTED_REPLY  0x20u

static uint8_t uart_spisend_received(void)
{
	static const char command[] = "SPISEND";
	static uint8_t index = 0;
	uint8_t byte = 0;

	while (usart_STM32.read_byte_nonblocking(&byte) != 0)
	{
		if (byte == (uint8_t)command[index])
		{
			index++;
			if (command[index] == '\0')
			{
				index = 0;
				return 1u;
			}
		}
		else
		{
			index = (byte == (uint8_t)command[0]) ? 1u : 0u;
		}
	}

	return 0u;
}

static char hex_digit(uint8_t value)
{
	value &= 0x0Fu;
	return (value < 10u) ? (char)('0' + value) : (char)('A' + (value - 10u));
}

static void uart_write_hex_byte(uint8_t value)
{
	char text[5];

	text[0] = '0';
	text[1] = 'x';
	text[2] = hex_digit(value >> 4u);
	text[3] = hex_digit(value);
	text[4] = '\0';

	usart_STM32.write_str(text);
}

static void led_pc13_init(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

	GPIOC->MODER &= ~(3u << (13u * 2u));
	GPIOC->MODER |=  (1u << (13u * 2u));
	GPIOC->OTYPER &= ~(1u << 13u);
	GPIOC->OSPEEDR |= (3u << (13u * 2u));
	GPIOC->PUPDR &= ~(3u << (13u * 2u));

	GPIOC->BSRR = (uint32_t)(1u << (13u + 16u));
}

static void led_pc13_on(void)
{
	GPIOC->BSRR = (uint32_t)(1u << 13u);
}

static void led_pc13_off(void)
{
	GPIOC->BSRR = (uint32_t)(1u << (13u + 16u));
}

int main(void)
{
	uint8_t respuesta = 0;

	SYSCLK_STM32.init();
	usart_STM32.init_115200();
	spi1_STM32.init();
	led_pc13_init();


	while (1)
	{
		if (uart_spisend_received() != 0u)
		{
			if (spi1_STM32.send_then_read_byte(SPI_CMD_TEST, SPI_DUMMY_BYTE, &respuesta) == SPI_IF_OK)
			{
				if (respuesta == SPI_EXPECTED_REPLY)
				{
					led_pc13_on();
					usart_STM32.write_str("SPITESTOK\r\n");
				}
				else
				{
					led_pc13_off();
					usart_STM32.write_str("SPIERROR RX=");
					uart_write_hex_byte(respuesta);
					usart_STM32.write_str("\r\n");
				}
			}
			else
			{
				led_pc13_off();
				usart_STM32.write_str("SPITIMEOUT\r\n");
			}
		}
	}
}
