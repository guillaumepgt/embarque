/*
 * sun_tracker.c
 *
 * Created on: 5 avr. 2019
 * Author: Nirgal
 */

#include "sun_tracker.h"
#include "SolTrack.h"   //librairie de calcul de la position du soleil
#include "stm32g4_rtc.h"
#include "stm32g4_systick.h"
#include "secretary.h"
#include "servo.h"      // AJOUT : Nécessaire pour piloter le servomoteur
#include "stm32g4_adc.h" // AJOUT : Nécessaire pour l'ADC
#include "stm32g4_uart.h" // AJOUT : Nécessaire pour lire l'UART (Echap)

//Réglages de la librairie SolTrack :
#define USE_DEGREES                     1       // Input (geographic position) and output are in degrees
#define USE_NORTH_EQUALS_ZERO           1       // Azimuth: 0 = North, pi/2 (90deg) = East
#define COMPUTE_REFRACTION_EQUATORIAL   1       // Compute refraction-corrected equatorial coordinates (Hour angle, declination): 0-no, 1-yes
#define COMPUTE_DISTANCE                1       // Compute the distance to the Sun in AU: 0-no, 1-yes
#define ENABLE_DISPLAY                  1

static SolTrackLocation_t loc;
static void process_ms(void);
static void SUN_TRACKER_compute_sun_position(SolTrackTime_t * time, SolTrackPosition_t * pos, SolTrackRiseSet_t * riseSet);

// Variable d'état : 0 = Manuel, 1 = Auto
// On l'initialise à 1 (Auto) par défaut
static int mode = 1;

void SUN_TRACKER_init(void)
{
    BSP_RTC_init();
    BSP_ADC_init(); // AJOUT : Initialisation de l'ADC

    // AJOUT : Accélération du temps pour les tests (5mn par seconde)
    #define TIME_ACCELERATION (60*5)
    BSP_RTC_set_time_acceleration(TIME_ACCELERATION);

    //Données par défaut - position de l'ESEO - Angers
    loc.latitude = 47.4933;
    loc.longitude = -0.5508;
    loc.pressure = 101.3;
    loc.temperature = 283.0;

    //Il faut bien démarrer avec une horaire de départ.
    RTC_DateTypeDef currentDate;
    RTC_TimeTypeDef currentTime;
    currentDate.Date = 1;
    currentDate.Month = 4;
    currentDate.Year = 19;
    currentTime.Hours = 8;      //attention à renseigner l'heure GMT
    currentTime.Minutes = 0;
    currentTime.Seconds = 0;
    BSP_RTC_set_time(&currentTime);
    BSP_RTC_set_date(&currentDate);

    BSP_systick_add_callback_function(&process_ms);

    // Initialisation du servo
    SERVO_init();
}

volatile bool flag_1s = false;

static void process_ms(void)
{
    static uint16_t t_ms = 0;
    if(t_ms == 0)
    {
        t_ms = 1000;
        flag_1s = true;
    }
    t_ms--;
}

void SUN_TRACKER_process_main(void)
{
    SECRETARY_process_main();

    // --- 1. Gestion du changement de mode (Touche Echap) ---
    // On vérifie si un caractère est arrivé sur l'UART2
    uint8_t c = BSP_UART_getc(UART2_ID);
    if (c == 0x1B) // 0x1B est le code ASCII de la touche Echap (ESC)
    {
        // Bascule du mode : si 1 devient 0, si 0 devient 1
        if(mode == 1) {
            mode = 0; // Passage en MANUEL
            printf("\r\n>>> PASSAGE EN MODE MANUEL <<<\r\n");
        } else {
            mode = 1; // Passage en AUTO
            printf("\r\n>>> PASSAGE EN MODE AUTO <<<\r\n");
        }
    }

    // --- 2. Exécution du MODE MANUEL (Prioritaire et rapide) ---
    if (mode == 0)
    {
        // Lecture immédiate du potentiomètre (ne pas attendre la seconde !)
        uint16_t raw_value = BSP_ADC_getValue(ADC_1); // Valeur entre 0 et 4095

        // Mise à l'échelle : Conversion 0-4095 vers 0-100
        // On cast en uint32_t pour éviter le dépassement lors de la multiplication
        uint16_t position = (uint16_t)( ((uint32_t)raw_value * 100) / 4095 );

        // Sécurité
        if (position > 100) position = 100;

        SERVO_set_position(position);
    }

    // --- 3. Exécution du MODE AUTO (Cadencé à 1Hz) ---
    else if (mode == 1 && flag_1s)
    {
        flag_1s = false;

        SolTrackTime_t time;
        RTC_DateTypeDef currentDate;
        RTC_TimeTypeDef currentTime;
        BSP_RTC_get_time_and_date(&currentTime,&currentDate);

        time.year = (int32_t)(currentDate.Year) + 2000;
        time.month = (int32_t)(currentDate.Month);
        time.day = (int32_t)(currentDate.Date);
        time.hour = (int32_t)(currentTime.Hours);
        time.minute = (int32_t)(currentTime.Minutes);
        time.second = (double)(currentTime.Seconds);

        SolTrackPosition_t pos;
        SolTrackRiseSet_t riseSet;

        // Calculs solaires
        HAL_GPIO_WritePin(LED_GREEN_GPIO, LED_GREEN_PIN, GPIO_PIN_SET);
        SUN_TRACKER_compute_sun_position(&time, &pos, &riseSet);
        HAL_GPIO_WritePin(LED_GREEN_GPIO, LED_GREEN_PIN, GPIO_PIN_RESET);

        // --- Pilotage du servomoteur selon l'azimuth ---
        uint16_t position = 0;
        double azimuth = pos.azimuthRefract;

        if (azimuth < 90.0 || azimuth > 270.0) {
            // Côté NORD (Nuit ou hors zone de suivi) : retour à 0
            position = 0;
        } else {
            // Côté SUD (Est -> Ouest) : proportionnel
            // Plage Azimuth : 90 à 270 (soit une étendue de 180 degrés)
            position = (uint16_t)((azimuth - 90.0) * (100.0 / 180.0));
        }

        // Sécurité
        if (position > 100) position = 100;

        SERVO_set_position(position);
    }
}

static void SUN_TRACKER_compute_sun_position(SolTrackTime_t * time, SolTrackPosition_t * pos, SolTrackRiseSet_t * riseSet)
{
    // Compute rise and set times:
    SolTrack_RiseSet(time, &loc, pos, riseSet, 0.0, USE_DEGREES, USE_NORTH_EQUALS_ZERO);

    // Compute positions:
    SolTrack(time, &loc, pos, USE_DEGREES, USE_NORTH_EQUALS_ZERO, COMPUTE_REFRACTION_EQUATORIAL, COMPUTE_DISTANCE);

    #if ENABLE_DISPLAY
        printf("Date                : %02d %02d %4d\n", time->day, time->month, time->year);
        printf("Heure               : %02d:%02d:%.1f\n", time->hour, time->minute, time->second);
        printf("azimuth et altitude corrigees : %.2f degre   %.2f degre \n\n", pos->azimuthRefract, pos->altitudeRefract);
    #endif
}

void SUN_TRACKER_set_new_pos(double latitude, double longitude) {
    loc.latitude=latitude;
    loc.longitude=longitude;
}
