/*
 * secretary.c
 *
 *  Created on: 28 mars 2019
 *      Author: Nirgal
 */
#include "secretary.h"
#include "stm32g4_utils.h"
#include "stm32g4_uart.h"
#include "stm32g4_rtc.h"
#include <stdio.h>

#define BUFFER_SIZE	128
static void find_command(char * buffer);
static void echo_command(char * buffer);
static bool str_begin_with(char * string, char * begin);



void SECRETARY_process_main(void)
{
	static char buffer2[BUFFER_SIZE]={0};
	static uint8_t index = 0;
	char c;

	while(BSP_UART_data_ready(UART2_ID)) {
		c = BSP_UART_getc(UART2_ID);
		buffer2[index] = c;
		if(32 == c) {
			buffer2[index] = '\0';
			find_command(buffer2);
			index = 0;
		} else if(index < BUFFER_SIZE - 1) {
			index ++;
			printf(buffer2);
		}
	}
}

/**
* @brief 	Fonction privée qui analyse le contenu de la chaine string pour voir si elle commence par le contenu de la chaîne begin
* @param	string : la chaîne dans laquelle on recherche
* @param	begin : la chaîne que l'on recherche
* @note		begin doit être de taille inférieure à string
* @return	true si string commence par begin. false sinon.
*/
static bool str_begin_with(char* string, char* begin){
	bool ret = true;
	uint16_t i;

	for(i=0;begin[i]!=0;i++){
		 if (string[i]==0 || string[i]!=begin[i]){
			 ret=false;
			 break;
		 }
	}
	return ret;
}

static void echo_command(char * buffer)
{
	puts(buffer);
}


const char * weekday_str[7] = {"sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"};
const uint8_t month_duration[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static void find_command(char * buffer)
{
	RTC_TimeTypeDef currentTime;
	RTC_DateTypeDef currentDate;
	bool badstring, printdate, printtime;
	printdate = false;
	printtime = false;
	badstring = false; //On fait l'hypothèse que la chaine est correcte.

	if(str_begin_with(buffer, "setdate"))
	{
		if(		buffer[8] >= '0' && buffer[8] <= '9'
			&&	buffer[9] >= '0' && buffer[9] <= '9'
			&&	buffer[11] >= '0' && buffer[11] <= '9'
			&&	buffer[12] >= '0' && buffer[12] <= '9'
			&&	buffer[14] >= '0' && buffer[14] <= '9'
			&&	buffer[15] >= '0' && buffer[15] <= '9')
		{
			currentDate.Date = (buffer[8]-'0') * 10 + (buffer[9]-'0');
			currentDate.Month = (buffer[11]-'0') * 10 + (buffer[12]-'0');
			currentDate.Year = (buffer[14]-'0') * 10 + (buffer[15]-'0');
			if(currentDate.Month >= 1 && currentDate.Month <= 12 && currentDate.Date >= 1 && currentDate.Date <= month_duration[currentDate.Month] && currentDate.Year < 100)
			{
				BSP_RTC_set_date(&currentDate);
				printdate = true;
			}
			else
			{
				printf("date must be between 1 and 29,30 or 31 ; month must be between 1 and 12 ; year must be < 100 (write 19 for 2019 ; 20 for 2020...)\n");
				badstring = true;
			}
		}
		else
			badstring = true;
	}
	else if(str_begin_with(buffer, "settime"))
	{
		if(		buffer[8] >= '0' && buffer[8] <= '9'
			&&	buffer[9] >= '0' && buffer[9] <= '9'
			&&	buffer[11] >= '0' && buffer[11] <= '9'
			&&	buffer[12] >= '0' && buffer[12] <= '9'
			&&	buffer[14] >= '0' && buffer[14] <= '9'
			&&	buffer[15] >= '0' && buffer[15] <= '9')
		{
			currentTime.Hours = (buffer[8]-'0') * 10 + (buffer[9]-'0');
			currentTime.Minutes = (buffer[11]-'0') * 10 + (buffer[12]-'0');
			currentTime.Seconds = (buffer[14]-'0') * 10 + (buffer[15]-'0');
			if(currentTime.Hours < 24 && currentTime.Minutes < 60 && currentTime.Seconds < 60)
			{
				printtime = true;
				BSP_RTC_set_time(&currentTime);
			}
			else
				badstring = true;
		}
		else
			badstring = true;
		if(badstring)
			printf("hours must be < 24 ; minutes must be < 60 ; seconds must be < 60\n");
	}
	else if(str_begin_with(buffer, "getdate"))
	{
		BSP_RTC_get_time_and_date(&currentTime, &currentDate);
		printdate = true;
	}
	else if(str_begin_with(buffer, "gettime"))
	{
		BSP_RTC_get_time_and_date(&currentTime, &currentDate);
		printtime = true;
	}
	else if(str_begin_with(buffer, "help"))
	{
		printf("--- commands list with examples ---\n");
		printf("\"setdate 01/04/19\\n\" (do not forget the zeros '0') - set a new date\n");
		printf("\"settime 08:05:42\\n\" (do not forget the zeros '0') - set a new time\n");
		printf("\"getdate\\n\" - get the current date \n");
		printf("\"gettime\\n\" - get the current time \n");
		printf("\"help\\n\" - show this commands list \n");
	}
	else
		badstring = true;
	if(printdate)
		printf("We are the %s %02d/%02d/%02d.\n", weekday_str[currentDate.WeekDay], currentDate.Date, currentDate.Month, currentDate.Year);
	if(printtime)
		printf("The hour is %02d:%02d:%02d.\n", currentTime.Hours, currentTime.Minutes, currentTime.Seconds);
	if(badstring)
		printf("Bad command -> press help to list commands\n");
}
