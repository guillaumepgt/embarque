/**
 *******************************************************************************
 * @file 	main.c
 * @author 	jjo
 * @date 	Mar 29, 2024
 * @brief	Fichier principal de votre projet sur carte Nucléo STM32G431KB
 *******************************************************************************
 */

#include "config.h"
#include "stm32g4_sys.h"

#include "stm32g4_systick.h"
#include "stm32g4_gpio.h"
#include "stm32g4_uart.h"
#include "stm32g4_adc.h"
#include "stm32g4_utils.h"
#include "HC-SR04/stm32g4_hcsr04.h"

#include "button.h"
#include "led.h"

#include <stdio.h>

static void state_machine(void);
void check_button(void);
void check_day(uint16_t channel);
void process_test_button(GPIO_TypeDef* gpio, uint32_t pin);
void process_test_photoresistor(uint16_t adc_id);
void process_test_telemeter(GPIO_TypeDef * TRIG_GPIO, uint16_t TRIG_PIN, GPIO_TypeDef * ECHO_GPIO, uint16_t ECHO_PIN);

static volatile uint32_t t = 0;
static uint16_t seuil = 100;
static uint16_t distance = 0;
static uint16_t day = 1;

void process_ms(void)
{
	if(t)
		t--;
}

static uint16_t count = 0;

/**
  * @brief  Point d'entrée de votre application
  */
int main(void)
{
	/* Cette ligne doit rester la première de votre main !
	 * Elle permet d'initialiser toutes les couches basses des drivers (Hardware Abstraction Layer),
	 * condition préalable indispensable à l'exécution des lignes suivantes.
	 */
	HAL_Init();

	/* Initialisation des périphériques utilisés dans votre programme */
	BSP_GPIO_enable();
	BSP_UART_init(UART2_ID,115200);
	BUTTON_init(GPIOA, GPIO_PIN_5);
	BSP_ADC_init();
	LED_init();

	/* Indique que les printf sont dirigés vers l'UART2 */
	BSP_SYS_set_std_usart(UART2_ID, UART2_ID, UART2_ID);

	//process_test_button(GPIOA, GPIO_PIN_5);
	//process_test_photoresistor(...);
	//process_test_telemeter(GPIOA, GPIO_PIN_8, GPIOB, GPIO_PIN_4);

	/* Tâche de fond, boucle infinie, Infinite loop,... quelque soit son nom vous n'en sortirez jamais */
	while (1)
	{
		check_button();
		state_machine();
		check_day(0);
	}
}

void state_machine(void)
{
	typedef enum
	{
		INIT,
		SCAN,
		PASSE


	}state_e;

	static state_e state = INIT;
	static state_e previous_state = INIT;
	bool entry = (state!=previous_state)?true:false;	//ce booléen sera vrai seulement 1 fois après chaque changement d'état.
	previous_state = state;

	static uint8_t telemeter_id = 0;
	static uint16_t detection_threshold = 0;

	BSP_HCSR04_process_main();
	if(!t)
	{
		BSP_HCSR04_run_measure(telemeter_id);
		t = HCSR04_TIMEOUT;
	}

	// Récupération des évènements
	bool new_measure_event = (BSP_HCSR04_get_value(telemeter_id, &distance) == HAL_OK)?true:false;
	printf("Distance: %d mm, seuil : %d, jour : %d                                          \r",  distance, seuil, day);

	switch(state)
	{
		case INIT:
			//printf("INIT\r");
			BSP_systick_add_callback_function(&process_ms);
			BSP_HCSR04_add(&telemeter_id, GPIOA, GPIO_PIN_8, GPIOB, GPIO_PIN_4);
			state = SCAN;
			break;

		case SCAN:
			LED_set(LED_FLASH);
			//printf("SCAN\r");
			BSP_HCSR04_process_main();
			BSP_HCSR04_get_value(telemeter_id, &distance);
			if(t) break;

			BSP_HCSR04_run_measure(telemeter_id);
			t = HCSR04_TIMEOUT;
			if (distance<seuil) {
				state= PASSE;
			}
			break;

		case PASSE:
			LED_set(LED_ON);
			//printf("PASSE\r");
			BSP_HCSR04_process_main();
			BSP_HCSR04_get_value(telemeter_id, &distance);

			if(t) break;
			BSP_HCSR04_run_measure(telemeter_id);
			t = HCSR04_TIMEOUT;
			if (distance>seuil) state = SCAN;
			break;

        default:
        	break;
	}
}

void check_button()
{
	static bool pressed = false;
	button_event_t button_event;
	button_event = BUTTON_state_machine();
	if (button_event == BUTTON_EVENT_PRESS && !pressed) {
		seuil = distance*0.9;
		pressed=true;
		printf("Boutton appuyé");

	} else if (button_event != BUTTON_EVENT_PRESS) pressed=false;
}


void check_day(uint16_t channel)
{
	if (BSP_ADC_getValue(channel) < 2000) day = 0;
	else day = 1;
}
