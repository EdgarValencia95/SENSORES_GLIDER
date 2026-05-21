#ifndef SYSTEM_CLOCK_IF_H
#define SYSTEM_CLOCK_IF_H

#include <stdint.h>

/* callback tipo void(uint32_t) */
typedef void (*clk_delay_fn_t)(volatile uint32_t n);

/* callback tipo void(void) */
typedef void (*clk_void_fn_t)(void);

typedef struct {
    clk_void_fn_t   init;        // configura reloj del sistema
    clk_delay_fn_t  delay_loop;  // delay bruto
    clk_delay_fn_t  delay_ms;    // delay en ms
} system_clock_if_t;

/* ===== ESTA ES LA "INSTANCIA" QUE SE INYECTA ===== */
extern const system_clock_if_t SYSCLK_STM32;

#endif // SYSTEM_CLOCK_IF_H