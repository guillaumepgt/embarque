/*
 * sample_bank.h
 *
 *  Created on: 19 avr. 2021
 *      Author: S. Poiraud

 */

#ifndef SAMPLE_BANK_H_
#define SAMPLE_BANK_H_


	/*
	 * Ce fichier vise à rassembler des exemples de morceaux de code en langage C.
	 * Complétez cette banque d'exemple à votre guise pour pouvoir vous y référer en cas de besoin
	 */


	////////////////////////////////////////////////////////////////////
	//Dans un fichier header (.h) :


	//Inclusions publiques des autres modules logiciels nécessaires à l'interprétation des déclarations de ce header :
	#include <stdint.h>			//une librairie
	#include "config.h"	//un fichier header issu de nos sources

	//Définition des types publiques (structures / énumération)

	typedef enum
	{
		COLOR_BLACK,
		COLOR_RED,
		COLOR_BLUE,
		COLOR_GREEN
	}color_e;


	typedef struct
	{
		int16_t x0;
		int16_t y0;
		int16_t x1;
		int16_t y1;
		int16_t distance;
		char name[20];
		color_e color;
	}segment_t;




	//Définition des prototypes des fonctions publiques
	int16_t SAMPLE_BANK_compute_segment_size(segment_t * s);

	void SAMPLE_BANK_segment_set_name(segment_t * s, char * name);

	void SAMPLE_BANK_segment_set_color(uint8_t index, color_e color);

	////////////////////////////////////////////////////////////////////


#endif /* SAMPLE_BANK_H_ */
