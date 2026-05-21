#include "system_clock_if.h"
#include "system_clock_driver.h"

/* ===== ESTA ES LA "INSTANCIA" QUE SE INYECTA ===== */
const system_clock_if_t SYSCLK_STM32 = {
    .init       = SystemClock_Config,
    .delay_loop = Delay_loop,
    .delay_ms   = Delay_ms
};