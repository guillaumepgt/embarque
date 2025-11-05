/*
 * sample_bank.c
 *
 *  Created on: 19 avr. 2021
 *      Author: S. Poiraud
 */



/*
 * Ce fichier vise à rassembler des exemples de morceaux de code en langage C.
 * Complétez cette banque d'exemple à votre guise pour pouvoir vous y référer en cas de besoin
 */


//Dans un fichier C (.c) :


//Inclusions privées des autres modules logiciels nécessaires à l'interprétation des déclarations de ce fichier source :
#include "sample_bank.h"
#include <stdio.h>
#include <string.h>
#include "sun_tracker.h"	//inutile ici, mais c'est pour l'exemple
#include "math.h"

//définition des macros privées
#define SEGMENTS_NB		10

//Définition des types privés (structures / énumérations)
//...

//Définition des variables (donc nécessairement privées : AUCUNE déclaration de variable dans un fichier header)
static segment_t segments[SEGMENTS_NB];		//tableau de segments


//Définition du corps des fonctions

void SAMPLE_BANK_init(void)
{

	//parcours d'un tableau de segments, et initialisation de chaque champs pour chaque segment du tableau
	for(uint8_t i = 0; i<SEGMENTS_NB; i++)
	{
		segments[i].x0 = 0;
		segments[i].y0 = 0;
		segments[i].x1 = 0;
		segments[i].y1 = 0;
		segments[i].color = COLOR_BLACK;
		segments[i].distance = 0;
		sprintf(segments[i].name, "segment %d", i);
	}
}


//calcul de la taille d'un segment d'après les deux coordonnées de ses extrémités.
int16_t SAMPLE_BANK_compute_segment_size(segment_t * s)
{
	double x, y, p;
	x = s->x1-s->x0;
	y = s->y1-s->y0;
	p = x*x+y*y;
	s->distance = (int16_t)(sqrt(p));
	return s->distance;
}

//Modification du nom du segment dont l'adresse est fournie en paramètre
void SAMPLE_BANK_segment_set_name(segment_t * s, char * name)
{
	uint8_t i;
	for(i=0; i<20 && name[i]!='\0'; i++)
		s->name[i] = name[i];
	s->name[i] = '\0';


	//équivaut à :
	sprintf(s->name, "%s", name);

	//équivaut à :
	strcpy(s->name, name);
}


//mise à jour de la couleur du segment de notre tableau dont l'index est fourni en paramètre
void SAMPLE_BANK_segment_set_color(uint8_t index, color_e color)
{
	if(index < SEGMENTS_NB)	//vérification de principe ! --> jamais d'accès hors tableau !
		segments[index].color = color;
}


char * SAMPLE_BANK_color_to_string(color_e color)
{
	char * ret = "unknown";
	switch(color)
	{
		case COLOR_BLACK:
			ret = "black";
			break;
		case COLOR_BLUE:
			ret = "blue";
			break;
		case COLOR_GREEN:
			ret = "green";
			break;
		case COLOR_RED:
			ret = "red";
			break;
		default:
			break;
	}
	return ret;
}


void SAMPLE_BANK_display_segment(segment_t * s)
{
	printf("The segment \"%s\" is going from [%d;%d] to [%d;%d].\n", s->name, s->x0, s->y0, s->x1, s->y1);
	printf("It measures %d and is configured with color %s.", s->distance, SAMPLE_BANK_color_to_string(s->color));
}

