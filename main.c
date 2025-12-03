/**
 *******************************************************************************
 * @file 	main.c
 * @author 	jjo
 * @date 	Mar 29, 2024
 * @brief	Fichier principal de votre projet sur carte Nucléo STM32G431KB
 *******************************************************************************
 */

#include "servo.h"
#include "config.h"
#include "stm32g4_sys.h"

#include "stm32g4_systick.h"
#include "stm32g4_gpio.h"
#include "stm32g4_uart.h"
#include "stm32g4_utils.h"

#include <stdio.h>

#define PERIOD_TIMER   10

void US_echo_falling_edge(uint8_t pin_number);

/**
 * @brief Fonction de callback sur front montant
 *
 * @param pin : numéro de la broche recevant le signal écho
 */
void US_echo_rising_edge(uint8_t pin_number)
{
	HAL_GPIO_TogglePin(LED_GREEN_GPIO, LED_GREEN_PIN);
}

/**
 * @brief Fonction de callback sur front descendant
 *
 * @param pin : numéro de la broche recevant le signal écho
 */
void US_echo_falling_edge(uint8_t pin_number)
{
	HAL_GPIO_WritePin(LED_GREEN_GPIO, LED_GREEN_PIN, GPIO_PIN_RESET);
	BSP_GPIO_pin_config(GPIOB, GPIO_PIN_4, GPIO_MODE_IT_RISING, GPIO_PULLDOWN, GPIO_SPEED_FREQ_HIGH,GPIO_NO_AF);
	BSP_EXTIT_set_callback(&US_echo_rising_edge, BSP_EXTIT_gpiopin_to_pin_number(GPIO_PIN_4), true);
}

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

	/* Indique que les printf sont dirigés vers l'UART2 */
	BSP_SYS_set_std_usart(UART2_ID, UART2_ID, UART2_ID);

	/* Configuration de la broche de la led Verte (carte Nucleo) */
	BSP_GPIO_pin_config(LED_GREEN_GPIO, LED_GREEN_PIN, GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_VERY_HIGH,GPIO_NO_AF);

	/* Configuration de la broche PA8 */
	BSP_GPIO_pin_config(GPIOA, GPIO_PIN_8, GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_VERY_HIGH,GPIO_NO_AF);

	BSP_TIMER_run_us(TIMER1_ID, PERIOD_TIMER*1000, false);
	BSP_TIMER_enable_PWM(TIMER1_ID, TIM_CHANNEL_1, 150, false, false);
	/* Tâche de fond, boucle infinie, Infinite loop,... quelque soit son nom vous n'en sortirez jamais */
	while (1)
	{

	}
}
