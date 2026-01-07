/*
 * button.h
 *
 *  Created on: 26 juin 2019
 *      Author: Nirgal
 *  Modifié
 *
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "stm32g4_utils.h"
#include "stm32g4_gpio.h"

typedef enum
{
	BUTTON_EVENT_NONE,
	BUTTON_EVENT_PRESS
}button_event_t;

void BUTTON_init(GPIO_TypeDef* gpio, uint32_t pin);

button_event_t BUTTON_state_machine(void);


#endif /* BUTTON_H_ */
