#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include <stdint.h>

void usart3_pb10_pb11_init_115200(void);
void usart3_write_byte(uint8_t b);
void usart3_write_str(const char *s);
int usart3_read_byte_nonblocking(uint8_t *out);
uint8_t usart3_read_byte_blocking(void);
void uart3_flush_tc(void);

#endif // UART_DRIVER_H