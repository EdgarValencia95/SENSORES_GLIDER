#include "uart_if.h"
#include "uart_driver.h"

const usart_if_t usart_STM32 = {
    .init_115200 = usart3_pb10_pb11_init_115200,
    .write_byte = usart3_write_byte,
    .write_str = usart3_write_str,
    .read_byte_nonblocking = usart3_read_byte_nonblocking,
    .read_byte_blocking = usart3_read_byte_blocking, 
    .flush_tc = uart3_flush_tc
};






