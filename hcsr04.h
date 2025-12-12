/*
 * hcsr04.h
 */
#ifndef HCSR04_H_
#define HCSR04_H_

#include <stdint.h>

// Initialise les broches et les interruptions
void HCSR04_init(void);

// Retourne la dernière distance mesurée en mm
uint32_t HCSR04_get_distance(void);

#endif /* HCSR04_H_ */