#ifndef SYSTEM_CLOCK_DRIVER_H
#define SYSTEM_CLOCK_DRIVER_H
#include <stdint.h>

void SystemClock_Config(void);
void Delay_loop(volatile uint32_t nCount);
void Delay_ms(volatile uint32_t ms);

#endif