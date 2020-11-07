/*
 * ctrl.c
 *
 *  Created on: 25 sept. 2019
 *      Author: Etud
 */

#include "ctrl.h"


#define	SPEED_KP			6		// Means KP = 6
#define SPEED_KI			3		// Means KI = 1/3
#define SPEED_LIMIT 		800		// Maximum PWM duty-cycle (*1000)
#define SPEED_INT_LIMIT		4000	// Integration clamping +/- (before KI)

//#define STEERING_KP		18
#define STEERING_KI			0.33
//#define STEERING_KD		1

#define DELTA_TRESHOLD		50
#define g_steering_trim		-110
#define CnV0                2250


// Global variables
xTaskHandle			vTaskCTRL_handle;


void vTaskCTRL (void *pvParameters)
{
	uint8_t		w;
	//uint8_t		i;

	uint8_t			pcam[128];
	//int16_t		delta;

	uint8_t		left_lane, right_lane;

	//int16_t		position_temp;



	uint16_t	enc_count[2], enc_count_prev[2]; //roue codeuse
	int32_t		speed_cmd_temp[2];

	uint16_t CnV1;


	my_printf("Entering Task CTRL\r\n");

	// Initialize controller variables
	for (w=0; w<2; w++)
	{
		enc_count[w]	  = 0;
		enc_count_prev[w] = 0;
		g_speed[w]		  = 0;
		g_speed_int[w]	  = 0;
	}

	g_steering=0;
	g_error_steering=0;
	g_error_n=0;
    g_delta=0;
    g_steering_kp=15;
    int right_white=0;
    int left_white=0;

	while(1)
	{
//-------------------Speed Regulation ------------------//
		// Read both speed encoders
		enc_count[0] = TPM2->CNT;		// 0 -> Left

		LPTMR0->CNR  = 0x5555;
		enc_count[1] = LPTMR0->CNR;		// 1 -> Right

		// Run speed controllers (PI) for both wheels (0->Left, 1->Right)
		for(w=0; w<2; w++)
		{
//ICI			// Compute Speed
			if (enc_count[w] >= enc_count_prev[w]) g_speed[w] = enc_count[w] - enc_count_prev[w];
			else g_speed[w] = 65536 + (enc_count[w] - enc_count_prev[w]);

			// Update counters
			enc_count_prev[w] = enc_count[w];

			// Update speed set point
			g_speed_sp[w] = g_speed_com_sp;			// Same speed for both wheels

			// Compute instantaneous g_error_steering
			g_speed_err[w]  = g_speed_sp[w]  - g_speed[w];

			// Compute integrated g_error_steering
			if (g_speed_com_sp > 5)					// Start integration only when moving, otherwise reset to zero
			{
				// Sum-up speed g_error_steering
				g_speed_int[w] = g_speed_int[w] + g_speed_err[w];

				// Clamp integration
				if (g_speed_int[w] >=  SPEED_INT_LIMIT) g_speed_int[w] =  SPEED_INT_LIMIT;
				if (g_speed_int[w] <= -SPEED_INT_LIMIT) g_speed_int[w] = -SPEED_INT_LIMIT;
			}
			else g_speed_int[w] = 0;

			// Compute PI command
			speed_cmd_temp[w]  = (g_speed_err[w]  * SPEED_KP) + (g_speed_int[w]  / SPEED_KI);

			// Check limits and apply command
			if ( (speed_cmd_temp[w] <= SPEED_LIMIT) & (speed_cmd_temp >= 0) )
			{
				g_speed_cmd[w] = speed_cmd_temp[w];
			}
			if (speed_cmd_temp[w] >  SPEED_LIMIT) 	g_speed_cmd[w] =  SPEED_LIMIT;
			if (speed_cmd_temp[w] <  0          )	g_speed_cmd[w] =  0;
		}


		// Update throttle
		if ((g_speed_cmd[0] + g_speed_cmd[1]) >0)				// Move forward
		{
			TPM0->CONTROLS[1].CnV = (uint16_t)g_speed_cmd[0];
			TPM0->CONTROLS[2].CnV = (uint16_t)0;

			TPM0->CONTROLS[4].CnV = (uint16_t)0;
			TPM0->CONTROLS[5].CnV = (uint16_t)g_speed_cmd[1];
		}

		if ((g_speed_cmd[0] + g_speed_cmd[1]) <0)				// Move backwards
		{
			TPM0->CONTROLS[1].CnV = (uint16_t)0;
			TPM0->CONTROLS[2].CnV = (uint16_t)(-g_speed_cmd[0]);

			TPM0->CONTROLS[4].CnV = (uint16_t)(-g_speed_cmd[1]);
			TPM0->CONTROLS[5].CnV = (uint16_t)0;
		}

		if ((g_speed_cmd[0] + g_speed_cmd[1]) == 0)				// Stop
		{
			TPM0->CONTROLS[1].CnV = 0;
			TPM0->CONTROLS[2].CnV = 0;

			TPM0->CONTROLS[4].CnV = 0;
			TPM0->CONTROLS[5].CnV = 0;
			delay_ms(1000);

		}

//---------------Position regulation--------------------//


		//Take the camera buffer
		for (int i=0; i<128; i++)
			{
			pcam[i] = waveform[i];
			}



		//Begin from the center of the camera then take the first right and left values which represents the black line
		right_white=0;
		left_white=0;
		while (*(pcam+63+right_white)>valWhite && right_white<=63)
			right_white++;
		right_lane=64+right_white;
		while (*(pcam+63-left_white)>valWhite && left_white<=63)
			left_white++;
		left_lane=64-left_white;

		//Evaluates the error between the track's center and the camera's center
		g_error_steering=((left_lane+right_lane)/2) - 64 ;
		// Beyond 14 pixels , we consider that the car needs to brack of its maximum value
		if (g_error_steering<-14){
			g_error_steering=-14;
		}
		if (g_error_steering>14){
			g_error_steering=14;
		}

		//If no lines detected, the error is the same (error must be big otherwise carrefour not passed)
		//if (right_lane<1 && left_lane>127 && g_error_n>10)
		//	g_error_steering=g_error_n;

		//Integrate the error; max value 14 min value -14
		   g_steering = g_steering + g_error_steering;
	    if (g_steering<-4)
	    	g_steering=-4;
	    if (g_steering>4)
	    	g_steering=4;


	    if (abs(g_error_steering)<2)
	    	g_steering_kp=11;
	    else  if (abs(g_error_steering)<4)
	    	g_steering_kp=12;
	    else  if (abs(g_error_steering)<6)
	    	g_steering_kp=13;
	    else  if (abs(g_error_steering)<8)
	    	g_steering_kp=14;
	    else if (abs(g_error_steering)<10)
	    	g_steering_kp=15;
	    else  if (abs(g_error_steering)<12)
	    	g_steering_kp=16;
	    else  if (abs(g_error_steering)<14)
	    	g_steering_kp=17;
	    else g_steering_kp=18;

		if (g_error_steering==0)
			g_speed_com_sp=65;
		else if (abs(g_error_steering)<=4)
				g_speed_com_sp=60;
		else if (abs(g_error_steering)<=10)
				g_speed_com_sp=55;
		else if (abs(g_error_steering)<=14)
				g_speed_com_sp=45;

		//Black on 3 pixels in front of the camera then stops after 500ms
		if (*(pcam+63)<valWhite){//(*(pcam+63)<valWhite && *(pcam+63-1)<valWhite && *(pcam+63+1)<valWhite){
			g_speed_com_sp=0;
			BSP_LED_On(1);
		}


		CnV1 = g_steering_trim + CnV0 - (g_error_steering*(g_steering_kp) + g_steering*STEERING_KI);

		if (TPM1->CONTROLS[0].CnV!=CnV1)
			TPM1->CONTROLS[0].CnV = CnV1;

		g_error_n = g_error_steering;
		g_lineR=right_lane;
		g_lineL=left_lane;
		// 50Hz update
		vTaskDelay(20);
	}
}


