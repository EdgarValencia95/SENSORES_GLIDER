#ifndef TMC5160_DRIVER_H_
#define TMC5160_DRIVER_H_

#include <stdint.h>

/////////////////////////////////////////Declaracion Funciones///////////////////////////////////////////////////

void tmc5160_pins_init(void);
void tmc5160_config(void);

void tmc5160_enable(void);
void tmc5160_disable(void);

void tmc5160_dir_clockwise(void);
void tmc5160_dir_counterclockwise(void);

void tmc5160_step_pulse(uint32_t delay_count);
void tmc5160_set_microstep_resolution(uint16_t MicroSteps);

#endif /* TMC5160_DRIVER_H_ */
