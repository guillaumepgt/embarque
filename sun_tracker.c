/*
 * sun_tracker.c
 *
 * Created on: 5 avr. 2019
 * Author: Nirgal
 */

#include "sun_tracker.h"
#include "SolTrack.h"	//librairie de calcul de la position du soleil
#include "stm32g4_rtc.h"
#include "stm32g4_systick.h"
#include "secretary.h"
#include "servo.h"      // AJOUT : Nécessaire pour piloter le servomoteur
#include "secretary.h"


//Réglages de la librairie SolTrack :
#define	USE_DEGREES						1		// Input (geographic position) and output are in degrees
#define USE_NORTH_EQUALS_ZERO			1		// Azimuth: 0 = North, pi/2 (90deg) = East
#define COMPUTE_REFRACTION_EQUATORIAL	1		// Compute refraction-corrected equatorial coordinates (Hour angle, declination): 0-no, 1-yes
#define	COMPUTE_DISTANCE				1		// Compute the distance to the Sun in AU: 0-no, 1-yes
#define ENABLE_DISPLAY 					1

static SolTrackLocation_t loc;
static void process_ms(void);
static void SUN_TRACKER_compute_sun_position(SolTrackTime_t * time, SolTrackPosition_t * pos, SolTrackRiseSet_t * riseSet);

int mode;

void SUN_TRACKER_init(void)
{
	BSP_RTC_init();

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
	currentTime.Hours = 8;		//attention à renseigner l'heure GMT
	currentTime.Minutes = 0;
	currentTime.Seconds = 0;
	BSP_RTC_set_time(&currentTime);
	BSP_RTC_set_date(&currentDate);

	BSP_systick_add_callback_function(&process_ms);
	mode = SECRETARY_get_mode();
    // Initialisation du servo (si ce n'est pas fait dans le main.c)
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
	if(flag_1s)
	{
		mode = SECRETARY_get_mode();

		//HAL_GPIO_TogglePin(LED_GREEN_GPIO, LED_GREEN_PIN);
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


		//variable qui serviront à receuillir les résultats des calculs
		SolTrackPosition_t pos;
		SolTrackRiseSet_t riseSet;

		//calculs
		HAL_GPIO_WritePin(LED_GREEN_GPIO, LED_GREEN_PIN, GPIO_PIN_SET);
		SUN_TRACKER_compute_sun_position(&time, &pos, &riseSet);
		HAL_GPIO_WritePin(LED_GREEN_GPIO, LED_GREEN_PIN, GPIO_PIN_RESET);

        // --- AJOUT : Pilotage du servomoteur selon l'azimuth ---
        uint16_t position = 0;
        double azimuth = pos.azimuthRefract;

        // Logique de positionnement
        if (mode==1){
			if (azimuth < 90.0 || azimuth > 270.0) {
				// Côté NORD (Nuit ou hors zone de suivi) : retour à 0
				position = 0;
			} else {
				// Côté SUD (Est -> Ouest) : proportionnel
				// Plage Azimuth : 90 à 270 (soit une étendue de 180 degrés)
				// Plage Servo : 0 à 100
				// Formule : (AzimuthActuel - OffsetEst) * (PlageServo / PlageAzimuth)
				position = (uint16_t)((azimuth - 90.0) * (100.0 / 180.0));
			}
        } else if (mode==0){
        	printf("test");
        }
        // Sécurité pour ne pas dépasser 100 (bien que SERVO_set_position le gère aussi)
        if (position > 100) position = 100;

        SERVO_set_position(position);
        // -------------------------------------------------------
	}
}

static void SUN_TRACKER_compute_sun_position(SolTrackTime_t * time, SolTrackPosition_t * pos, SolTrackRiseSet_t * riseSet)
{
	// Compute rise and set times:
	SolTrack_RiseSet(time, &loc, pos, riseSet, 0.0, USE_DEGREES, USE_NORTH_EQUALS_ZERO);

	// Compute positions:
	SolTrack(time, &loc, pos, USE_DEGREES, USE_NORTH_EQUALS_ZERO, COMPUTE_REFRACTION_EQUATORIAL, COMPUTE_DISTANCE);
	#if !ENABLE_DISPLAY
		printf("Date                : %02d %02d %4d\n", time->day, time->month, time->year);
		printf("Heure               : %02d:%02d:%.1f\n", time->hour, time->minute, time->second);
        // Le reste des printf...
		printf("azimuth et altitude corrigees : %.2f degre   %.2f degre \n\n", pos->azimuthRefract, pos->altitudeRefract);
	#endif

}

void SUN_TRACKER_set_new_pos(double latitude, double longitude) {
	loc.latitude=latitude;
	loc.longitude=longitude;
}
