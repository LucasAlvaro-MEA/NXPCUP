/*
 * main.h
 *
 *  Created on: 23 févr. 2019
 *      Author: Laurent
 */

#ifndef MAIN_H_
#define MAIN_H_

// Device header
#include "MKL25Z4.h"

// FreeRTOS headers
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"
#include "stream_buffer.h"

// BSP headers
#include "bsp.h"
#include "mma.h"
#include "oled.h"

// APP headers
#include "delay.h"
#include "cam.h"
#include "accl.h"
#include "ctrl.h"
#include "display.h"


#define	EXPO_NCLK		200
#define	EXPO_NCLK_X2	400

extern int my_printf(const char *format, ...);
extern int my_sprintf(char *out, const char *format, ...);


// External global variables made available to all functions

extern uint16_t		g_adc_key,
					g_lineR,
					g_lineL;

extern	int16_t		g_position;
extern int8_t		g_error_steering,
					valWhite;


extern uint16_t		g_steering_trim;		// Used to set zero streering PWM
extern int16_t		g_steering;// Steering command

extern int32_t		g_speed_com_sp;				// Common speed set point

extern int32_t		g_speed_sp[2],			// Speed set point 			(L/R)
					g_speed[2],				// Actual speed 			(L/R)
					g_speed_err[2],			// Speed error				(L/R)
					g_speed_int[2],			// Speed integrated error	(L/R)
					g_speed_cmd[2],		// Speed command			(L/R)
					g_delta,               // Delta error
					g_error_n,
					g_steering_kp;



extern	uint16_t	g_cam_expo;				// Camera exposition
extern uint16_t*	g_pcam_buffer;			// Pointer to valid camera buffer

#endif /* MAIN_H_ */
