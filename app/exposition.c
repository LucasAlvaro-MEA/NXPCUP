/*
 * exposition.c
 *
 *  Created on: 22 oct. 2019
 *      Author: Etud
 */

#include "main.h"

//Measure the time exposition to have a color white between 110 and 140

void exposition_init(){

	uint8_t	time_integration;
	uint16_t*	pcam;

	//Take the camera buffer
	pcam = g_pcam_buffer;

	while (*(pcam+63)>110 && *(pcam+63)<140)
	{
		time_integration++;
	}


}

