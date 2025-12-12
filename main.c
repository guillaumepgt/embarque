/**
 * main.c
 */
#include "config.h"
#include "stm32g4_sys.h"
#include "stm32g4_gpio.h"
#include "stm32g4_uart.h"
#include "stm32g4_utils.h"
#include <stdio.h>

// Inclusion de nos modules
#include "hcsr04.h"
#include "servo.h" // Si vous l'utilisez encore

#define PERIOD_TIMER   100 // 100ms pour laisser le temps au capteur

int main(void)
{
    HAL_Init();
    BSP_GPIO_enable();
    BSP_UART_init(UART2_ID, 115200);
    BSP_SYS_set_std_usart(UART2_ID, UART2_ID, UART2_ID);

    // Init LED
    BSP_GPIO_pin_config(LED_GREEN_GPIO, LED_GREEN_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_NO_AF);

    // --- INIT DU TRIGGER (TIMER 1) ---
    // Envoie une impulsion sur PA8 toutes les 100ms
    BSP_GPIO_pin_config(GPIOA, GPIO_PIN_8, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_NO_AF);
    BSP_TIMER_run_us(TIMER1_ID, PERIOD_TIMER * 1000, false);
    BSP_TIMER_enable_PWM(TIMER1_ID, TIM_CHANNEL_1, 150, false, false);
    *(uint32_t*)0x40012c34 = 35; // ~35µs pulse width

    // --- INIT DU CAPTEUR (ECHO) ---
    HCSR04_init();



    while (1)
    {
        // On récupère la distance mesurée en tâche de fond par les interruptions
        uint32_t dist_mm = HCSR04_get_distance();

        // On l'affiche
        printf("Distance : %lu mm \r", dist_mm); // \r pour réécrire sur la même ligne

        HAL_Delay(200); // Pas besoin de spammer l'affichage
    }
}
