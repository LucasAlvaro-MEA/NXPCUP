/*
 * display.h
 *
 *  Created on: 9 mars 2019
 *      Author: Laurent
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "main.h"

void	vTaskDISP (void *pvParameters);

extern 	xTaskHandle			vTaskDISP_handle;
extern 	xSemaphoreHandle	xDISPSem;

extern 	uint8_t				waveform[128];


#endif /* DISPLAY_H_ */
