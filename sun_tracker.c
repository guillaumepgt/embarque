/*
 * sun_tracker.c
 *
 *  Created on: 5 avr. 2019
 *      Author: Nirgal
 */


#include "sun_tracker.h"
#include "SolTrack.h"	//librairie de calcul de la position du soleil
#include "stm32g4_rtc.h"
#include "stm32g4_systick.h"
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


void SUN_TRACKER_init(void)
{
	BSP_RTC_init();

	//Données par défaut - position de l'ESEO - Angers
	loc.latitude = 47.4933;
	loc.longitude = -0.5508;
	loc.pressure = 101.3;		//En haut du batiment, on est plus haut qu'en bas... mais globalement pas loin du niveau de la mer.
								//on néglige les effets de l'altitude
	loc.temperature = 283.0;  	// Atmospheric temperature in K

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
		HAL_GPIO_TogglePin(LED_GREEN_GPIO, LED_GREEN_PIN);
		flag_1s = false;
		SolTrackTime_t time;
		//données de test temporaires.
		time.year = 	(int32_t)(20) + 2000;	//2020
		time.month = 	(int32_t)(4);			//avril
		time.day = 		(int32_t)(1);			//1er
		time.hour = 	(int32_t)(8);
		time.minute = 	(int32_t)(30);
		time.second = 	(double)(40.4);


		//variable qui serviront à receuillir les résultats des calculs
		SolTrackPosition_t pos;
		SolTrackRiseSet_t riseSet;

		//calculs
		SUN_TRACKER_compute_sun_position(&time, &pos, &riseSet);
	}
}


static void SUN_TRACKER_compute_sun_position(SolTrackTime_t * time, SolTrackPosition_t * pos, SolTrackRiseSet_t * riseSet)
{
	// Compute rise and set times:
	SolTrack_RiseSet(time, &loc, pos, riseSet, 0.0, USE_DEGREES, USE_NORTH_EQUALS_ZERO);

	// Compute positions:
	SolTrack(time, &loc, pos, USE_DEGREES, USE_NORTH_EQUALS_ZERO, COMPUTE_REFRACTION_EQUATORIAL, COMPUTE_DISTANCE);
	#if ENABLE_DISPLAY
		printf("Date                : %2d %2d %4d\n", time->day, time->month, time->year);
		printf("Heure               : %d:%d:%.1f\n", time->hour, time->minute, time->second);
		printf("Jour Julien         : %.2f\n\n", pos->julianDay);

		printf("Heure leve  : %6.4f,   azimuth : %.2f\n", riseSet->riseTime, riseSet->riseAzimuth);
		printf("Heure zenith: %6.4f,   altitude: %.2f\n", riseSet->transitTime, riseSet->transitAltitude);
		printf("Heure couche: %6.4f,   azimuth : %.2f\n\n", riseSet->setTime, riseSet->setAzimuth);

		printf("Ecliptique longitude          : %.2f degre \n", pos->longitude);
		printf("ascension droite et declinaison: %.2f degre   %.2f degre \n", pos->rightAscension, pos->declination);
		printf("altitude non corrigee         : %.2f degre\n\n", pos->altitude);
		printf("azimuth et altitude corrigees : %.2f degre   %.2f degre \n\n", pos->azimuthRefract, pos->altitudeRefract);
	#endif

}

void SUN_TRACKER_set_new_pos(double latitude, double longitude) {
	loc.latitude=latitude;
	loc.longitude=longitude;
}
