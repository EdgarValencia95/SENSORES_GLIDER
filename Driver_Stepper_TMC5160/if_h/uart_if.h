#ifndef UART_IF_H
#define UART_IF_H
#include <stdint.h>

typedef struct {
    void (*init_115200)(void);
    void (*write_byte)(uint8_t b);
    void (*write_str)(const char *s);
    int (*read_byte_nonblocking)(uint8_t *out);
    uint8_t (*read_byte_blocking)(void);
    void (*flush_tc)(void);
}usart_if_t;

extern const usart_if_t usart_STM32;
#endif