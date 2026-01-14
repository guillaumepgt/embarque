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
static uint16_t distance = 0;
static bool day = 1;

static button_event_t button_event;

void process_ms(void)
{
	if(t)
		t--;
}


/**
  * @brief  Point d'entrée de votre application
  */
int main(void)
{

	HAL_Init(); //Initialisation de la bibliothèque HAL

	BSP_GPIO_enable(); //Activation de l'horloge des ports GPIO
	BSP_UART_init(UART2_ID,115200); //Initialisation de l'UART2 à 115200 bauds pour l'affichage
	BUTTON_init(GPIOA, GPIO_PIN_5); //Initialisation du bouton (pin A5)
	BSP_ADC_init(); //Initialisation de l'ADC
	LED_init();    //Initialisation de la LED verte (pin B8)

	BSP_SYS_set_std_usart(UART2_ID, UART2_ID, UART2_ID); //Redirection de la sortie standard sur l'UART2

	//Boucle principale
	while (1)
	{
		button_event = BUTTON_state_machine(); //Récupération des événements du bouton
		state_machine();  //Machine à états principale
		check_day(0);  //Vérification du jour/nuit via la photo-résistance
	}
}
// Machine à états principale
void state_machine(void)
{
	typedef enum //Définition des états possibles
	{
		INIT,
		INSTALL,
		SCAN,
		PASSE,
		NUIT,
		STOP


	}state_e;

	static state_e state = INIT; //On initialise l'état INIT
	static state_e previous_state = INIT; //Variable pour stocker l'état précédent
	previous_state = state; //Mise à jour de l'état précédent

	static uint8_t telemeter_id = 0; //Identifiant du télémètre HC-SR04 (au cas où on en aurait plusieurs)
	static uint16_t count = 0;  //Compteur de personnes
	static uint8_t seuil = 100; //Seuil de détection (en mm, valeur arbitraire puisqu'elle sera calibrée lors de l'installation)
	static uint8_t seuilPersonnes = 100; //Seuil de personnes avant arrêt (valeur arbitraire)

	BSP_HCSR04_process_main(); //Traitement principal des télémètres HC-SR04
	if(!t)
	{
		BSP_HCSR04_run_measure(telemeter_id); //Lancement d'une nouvelle mesure
		t = HCSR04_TIMEOUT; //Remise à zéro du timer
	}

	bool new_measure_event = (BSP_HCSR04_get_value(telemeter_id, &distance) == HAL_OK)?true:false; //Vérification de la disponibilité d'une nouvelle mesure

	switch(state)
	{
		case INIT: // Cas INIT: initialisation des modules
			BSP_systick_add_callback_function(&process_ms); 
			BSP_HCSR04_add(&telemeter_id, GPIOA, GPIO_PIN_8, GPIOB, GPIO_PIN_4);//Initialisation d'un télémètre HC-SR04
			state = INSTALL; //On passe à l'état INSTALL
			break;

		case INSTALL: // Cas INSTALL: calibration du seuil de détection
			LED_set(LED_BLINK); //Clignotement lent de la LED pour indiquer le mode d'installation
			BSP_HCSR04_process_main(); //Traitement principal des télémètres HC-SR04
			BSP_HCSR04_get_value(telemeter_id, &distance); //Récupération de la dernière mesure

			BSP_HCSR04_run_measure(telemeter_id); //Lancement d'une nouvelle mesure
			t = HCSR04_TIMEOUT; //Remise à zéro du timer
			if (button_event == BUTTON_EVENT_SHORT_PRESS){
				state = SCAN; /// On passe à l'état SCAN si le bouton est pressé court
				seuil = distance; //On enregistre la distance mesurée comme seuil de détection
			}
			break;

		case SCAN:  // Cas SCAN: attente de detection d'un passage
			LED_set(LED_FLASH); //Clignotement rapide de la LED pour indiquer le mode scan
			BSP_HCSR04_process_main();
			BSP_HCSR04_get_value(telemeter_id, &distance);
			if(t) break;

			BSP_HCSR04_run_measure(telemeter_id);
			t = HCSR04_TIMEOUT;
			if (distance<seuil) {  //Si la distance mesurée est inférieure au seuil, on considère qu'une personne est en train de passer
				state= PASSE; //On passe à l'état PASSE
			}
			else if (button_event == BUTTON_EVENT_LONG_PRESS){ //Si le bouton est pressé longuement, on retourne en mode INSTALL
				state = INSTALL; //on passe à l'état INSTALL
			}
			else if (!day) state = NUIT; //Si c'est la nuit, on passe à l'état NUIT
			break;

		case PASSE: // Cas PASSE: une personne est en train de passer, on attent qu'elle soit complètement passée
			LED_set(LED_ON); //Allumage fixe de la LED pour indiquer le mode passage
			BSP_HCSR04_process_main(); 
			BSP_HCSR04_get_value(telemeter_id, &distance);

			if(t) break;
			BSP_HCSR04_run_measure(telemeter_id);
			t = HCSR04_TIMEOUT;
			if (distance>seuil) { //Si la distance mesurée est supérieure au seuil, on considère que la personne est passée
				count++; //On incrémente le compteur de personnes
				if (count<seuilPersonnes) state = SCAN; //Si le compteur est inférieur au seuil, on retourne en mode SCAN
				else state = STOP; //Sinon, on passe à l'état STOP (on a atteint le nombre de personnes maximum)
			}
			break;

		case NUIT: // Cas NUIT: mode basse consommation la nuit (on considère qu'il n'y a pas de passage)
			LED_set(LED_OFF); //On éteint la LED pour indiquer le mode nuit
			if (day) state = SCAN; //Si c'est le jour, on retourne en mode SCAN
			else if (button_event == BUTTON_EVENT_LONG_PRESS) { //Si le bouton est pressé longuement, on retourne en mode INSTALL
				state = INSTALL; //on passe à l'état INSTALL
				count=0; //reset du compteur
			}
			break;

		case STOP: // Cas STOP: arrêt du comptage après avoir atteint le seuil de personnes
			LED_set(LED_OFF); //On éteint la LED pour indiquer le mode arrêt
			if (button_event == BUTTON_EVENT_SHORT_PRESS){ //Si le bouton est pressé court, on retourne en mode SCAN
				state = SCAN; //on passe à l'état SCAN
				count=0; //reset du compteur
			} else if (button_event == BUTTON_EVENT_LONG_PRESS) { //Si le bouton est pressé longuement, on retourne en mode INSTALL
				state = INSTALL; //on passe à l'état INSTALL
				count=0; //reset du compteur
			}


        default: // Sécurité: retour à l'état INIT en cas d'état inconnu
			state = INIT; //On passe à l'état INIT
        	break;
	}
}



void check_day(uint16_t channel) //Vérification jour/nuit via la photo-résistance connectée à l'ADC
{
	if (BSP_ADC_getValue(channel) < 2000) day = 0; //Seuil pour déterminer si c'est la nuit
	else day = 1; //Sinon, c'est le jour
}
