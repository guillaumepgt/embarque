/*
 * button.c
 *
 *  Created on: 26 juin 2019
 *      Author: Nirgal
 */

#include "button.h"
#include "config.h"
#include "stm32g4_systick.h"
#include "stdio.h"

static void process_ms(void);

static volatile bool flag_10ms;
static volatile uint32_t t = 0;
static bool initialized = false;
static GPIO_TypeDef* button_gpio;
static uint32_t button_pin;

void BUTTON_init(GPIO_TypeDef* gpio, uint32_t pin)
{
	button_gpio = gpio;
	button_pin = pin;
	//Initialisation du port choisi en entrée pour le bouton
	BSP_GPIO_pin_config(button_gpio, button_pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH, GPIO_NO_AF);

	BSP_systick_add_callback_function(&process_ms);

	initialized = true;
}

static void process_ms(void)
{
	static uint32_t t10ms = 0;
	t10ms = (t10ms + 1)%10;		//incrémentation de la variable t10ms (modulo 10 !)
	if(!t10ms)
		flag_10ms = true; //toutes les 10ms, on lève ce flag.
	if(t)
		t--;
}

/**
	Cette machine à états gère la détection d'appuis sur le bouton bleu.
	Elle doit être appelée en boucle très régulièrement.
	Précondition : avoir appelé auparavant BUTTON_init();
	Si un appui vient d'être fait, elle renverra BUTTON_EVENT_SHORT_PRESS ou BUTTON_EVENT_LONG_PRESS
*/
button_event_t BUTTON_state_machine(void)
{
	typedef enum
	{
		INIT = 0,
		WAIT_BUTTON,	//En C, les nombres se suivent dans une enum.
		BUTTON_PRESSED,
		WAIT_RELEASE
	}state_e;

	static state_e state = INIT; //La variable d'état, = INIT au début du programme !
	/**	Le mot clé static est INDISPENSABLE :
	* 	"state" DOIT GARDER SA VALEUR d'un appel à l'autre de la fonction.
	*	Une place lui est réservée en mémoire de façon permanente
	*	(et non pas temporaire dans la pile !)
	*/

	button_event_t ret = BUTTON_EVENT_NONE;
	bool current_button;

	if(flag_10ms && initialized)	//le cadencement de cette portion de code à 10ms permet d'éliminer l'effet des rebonds sur le signal en provenance du bouton.
	{
		flag_10ms = false;
		current_button = !HAL_GPIO_ReadPin(button_gpio, button_pin);
		switch(state)
		{
			case INIT:
				state = WAIT_BUTTON;	//Changement d'état
				break;
			case WAIT_BUTTON:
				if(current_button)
				{
					printf("[BUTTON      ] button pressed\n");
					state = BUTTON_PRESSED;	//Changement d'état conditionné à "if(current_button)"
				}
				break;
			case BUTTON_PRESSED:
				if(current_button)
				{
					ret = BUTTON_EVENT_PRESS;
				}
				else
				{
					state= WAIT_BUTTON;
				}
				break;

			default:
				state = INIT;	//N'est jamais sensé se produire.
				break;
		}
	}
	return ret;
}

