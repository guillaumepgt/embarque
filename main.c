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
#include "stm32g4_utils.h"
#include "sun_tracker.h"
#include "servo.h"

#include <stdio.h>

void useless_function(void);
void tab_cypher(void);

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

	/* Initialisation du port de la led Verte (carte Nucleo) */
	BSP_GPIO_pin_config(LED_GREEN_GPIO, LED_GREEN_PIN, GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_HIGH,GPIO_NO_AF);

	SUN_TRACKER_init();

	//useless_function();
	//while(!BSP_UART_button(UART2_ID));

	SERVO_init();
	SERVO_set_position(0);
	while(1)
	{
		SUN_TRACKER_process_main();
		//SERVO_process_test();
		//if (BSP_UART_button(UART2_ID)) {

		//}
	}

}


volatile uint32_t e;	//variable accessible depuis l'onglet expressions

//cette fonction ne sert à rien d'autre qu'à montrer le rôle des opérateurs en C et s'entrainer à utiliser le débogueur
void useless_function(void)
{
	static uint32_t n = 0;
	volatile uint8_t a = 0xCA;
	volatile bool b;
	volatile uint32_t d;
	volatile uint8_t c;

	b = 0b10101010 || 0b11110000;
	b = 0b10101010 && 0b11110000;
	b = !42;
	b = !0;

	c = 0b10101010 & 0b11110000;
	c = 0b10101010 ^ 0b11110000;
	c = 0b10101010 | 0b11110000;
	c = ~0b00001111;
	c ^= c;

	d = (0xFE << 16);
	d |= ((uint32_t)(a))<<24;
	d |= a;
	d += 0xDE << 8;

	e = 2976579765;

	while(1)
	{
		b = BSP_UART_button(UART2_ID);
		if(b)
		{
			n++;
		}
	}
}

#define TAB_SIZE        18

void tab_cypher(void)
{
        // Explorez étape par étape le parcours de ce tableau
        char my_super_tab[TAB_SIZE] = {'o','v','s','u','a',' ','e','v',' ','z','e','r','s','u','i','s','!','!'};
        uint8_t index = 0;

        for(index = 0; index <= TAB_SIZE; index++)
        {
                volatile char my_char = my_super_tab[index];
                BSP_UART_putc(UART2_ID, my_char);
        }
        printf(my_super_tab);
        BSP_UART_putc(UART2_ID,'\n');   // Un retour à la ligne pour faire propre

}
