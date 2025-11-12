#ifndef SERVO_H
#define SERVO_H

#include "config.h"
#include "stm32g4_timer.h"
#include "stm32g4_uart.h"
#include "stm32g4_utils.h"
#include <stdint.h>

void SERVO_init(void);
void SERVO_set_position(uint16_t position);
void SERVO_process_test(void);
uint16_t SERVO_get_position(void);

#endif
