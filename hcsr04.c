/*
 * hcsr04.c
 */
#include "hcsr04.h"
#include "config.h"
#include "stm32g4_utils.h"
#include "stm32g4_gpio.h"
#include "stm32g4_extit.h"
#include <stdio.h> // Pour le printf de debug si besoin

#define SOUND_SPEED 340 // Vitesse du son en m/s

// --- Variables Privées (Static) ---
static uint32_t pulse_start_timestamp = 0;
static uint32_t pulse_end_timestamp = 0;
static uint32_t distance = 0;

// --- Prototypes des fonctions internes ---
static void US_echo_rising_edge(uint8_t pin_number);
static void US_echo_falling_edge(uint8_t pin_number);

// --- Implémentation ---

void HCSR04_init(void)
{
    // 1. Configuration de PB4 en entrée avec interruption sur front montant
    BSP_GPIO_pin_config(GPIOB, GPIO_PIN_4, GPIO_MODE_IT_RISING, GPIO_PULLDOWN, GPIO_SPEED_FREQ_HIGH, GPIO_NO_AF);

    // 2. Branchement du callback initial
    BSP_EXTIT_set_callback(&US_echo_rising_edge, BSP_EXTIT_gpiopin_to_pin_number(GPIO_PIN_4), true);
}

uint32_t HCSR04_get_distance(void)
{
    return distance;
}

// --- Fonctions Interruptions (Callbacks) ---

static void US_echo_rising_edge(uint8_t pin_number)
{
    // 1. On enregistre l'heure de départ (en µs)
    pulse_start_timestamp = BSP_systick_get_time_us();

    // 2. Debug visuel (Optionnel : allume la LED)
    HAL_GPIO_WritePin(LED_GREEN_GPIO, LED_GREEN_PIN, GPIO_PIN_SET);

    // 3. On prépare la détection de la fin du pulse
    BSP_GPIO_pin_config(GPIOB, GPIO_PIN_4, GPIO_MODE_IT_FALLING, GPIO_PULLDOWN, GPIO_SPEED_FREQ_HIGH, GPIO_NO_AF);
    BSP_EXTIT_set_callback(&US_echo_falling_edge, BSP_EXTIT_gpiopin_to_pin_number(GPIO_PIN_4), true);
}

static void US_echo_falling_edge(uint8_t pin_number)
{
    // 1. On enregistre l'heure de fin
    pulse_end_timestamp = BSP_systick_get_time_us();

    // 2. Calcul de la durée
    uint32_t duration = pulse_end_timestamp - pulse_start_timestamp;

    // 3. Calcul de la distance : d = v * t / 2
    // Formule : (Temps_us * 340 m/s) / 2000 (pour convertir en mm et diviser par 2)
    // Protection contre les valeurs aberrantes (bruit ou timeout)
    if (duration < 30000) // On ignore si durée > 30ms (environ 5 mètres)
    {
        distance = (duration * SOUND_SPEED) / 2000;
    }

    // 4. Debug visuel (Optionnel : éteint la LED)
    HAL_GPIO_WritePin(LED_GREEN_GPIO, LED_GREEN_PIN, GPIO_PIN_RESET);

    // 5. On se remet en attente du prochain pulse
    BSP_GPIO_pin_config(GPIOB, GPIO_PIN_4, GPIO_MODE_IT_RISING, GPIO_PULLDOWN, GPIO_SPEED_FREQ_HIGH, GPIO_NO_AF);
    BSP_EXTIT_set_callback(&US_echo_rising_edge, BSP_EXTIT_gpiopin_to_pin_number(GPIO_PIN_4), true);
}
