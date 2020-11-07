/*
 * ctrl.c
 *
 *  Created on: 9 mars 2019
 *      Author: Laurent
 */
/*
#include "ctrl.h"


#define	SPEED_KP			6		// Means KP = 6
#define SPEED_KI			3		// Means KI = 1/3
#define SPEED_LIMIT 		800		// Maximum PWM duty-cycle (*1000)
#define SPEED_INT_LIMIT		4000	// Integration clamping +/- (before KI)

#define STEERING_KP			12
#define DELTA_TRESHOLD		60



// Global variables
xTaskHandle			vTaskCTRL_handle;



void vTaskCTRL (void *pvParameters)
{
	uint8_t		w;
	uint8_t		i;

	uint16_t*	pcam;
	int16_t		delta;

	uint8_t		left_lane, right_lane;

	int16_t		position_temp;

	uint16_t	enc_count[2], enc_count_prev[2];
	int32_t		speed_cmd_temp[2];



	my_printf("Entering Task CTRL\r\n");

	g_steering_trim = 2135;
	g_steering = 0;


	// Initialize controller variables
	for (w=0; w<2; w++)
	{
		enc_count[w]	  = 0;
		enc_count_prev[w] = 0;
		g_speed[w]		  = 0;
		g_speed_int[w]	  = 0;
	}

	while(1)
	{

		// Read both speed encoders
		enc_count[0] = TPM2->CNT;		// 0 -> Left

		LPTMR0->CNR  = 0x5555;
		enc_count[1] = LPTMR0->CNR;		// 1 -> Right

		// Run speed controllers (PI) for both wheels (0->Left, 1->Right)
		for(w=0; w<2; w++)
		{
			// Compute Speed
			if (enc_count[w] >= enc_count_prev[w]) g_speed[w] = enc_count[w] - enc_count_prev[w];
			else g_speed[w] = 65536 + (enc_count[w] - enc_count_prev[w]);

			// Update counters
			enc_count_prev[w] = enc_count[w];

			// Update speed set point
			g_speed_sp[w] = g_speed_com_sp;			// Same speed for both wheels

			// Compute instantaneous error
			g_speed_err[w]  = g_speed_sp[w]  - g_speed[w];

			// Compute integrated error
			if (g_speed_com_sp > 5)					// Start integration only when moving, otherwise reset to zero
			{
				// Sum-up speed error
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
		}



		// Calculate position across road

		i = 0;
		pcam = g_pcam_buffer;

		left_lane  = 255;
		right_lane = 255;


		// Search for left lane
		i = 64;
		while(i>0)
		{
			delta = *(pcam+i) - *(pcam+i-1);
			if(delta > DELTA_TRESHOLD) left_lane = i;
			i--;
		}

		// Search for right lane
		i=64;
		while(i<127)
		{
			delta = *(pcam+i) - *(pcam+i+1);
			if(delta > DELTA_TRESHOLD) right_lane = i;
			i++;
		}

		// Straight road
		if ((left_lane <128) & (right_lane <128))
		{
			BSP_LED_On(WHITE);

			position_temp = ((left_lane + right_lane)/2) - 65;

			if ((position_temp <20) & (position_temp >-20))
			{
				g_position = -position_temp;
			}
		}

		// Left turn
		if ((left_lane >128) & (right_lane <128))
		{
			BSP_LED_On(GREEN);
			g_position = 20;
		}

		// Right turn
		if ((left_lane <128) & (right_lane >128))
		{
			BSP_LED_On(RED);
			g_position = -20;
		}

		// Crossings
		if ((left_lane >128) & (right_lane >128))
		{
			BSP_LED_On(BLUE);
		}

		// Compute steering
		g_steering = STEERING_KP * g_position;


//		while(i<127)
//		{
//			// Compute derivative
//			delta = *(pcam + 1) - *pcam;
//
//			// Search for min and max
//			if (delta > delta_max)
//			{
//				delta_max = delta;
//				i_delta_max = i;
//			}
//
//			if (delta < delta_min)
//			{
//				delta_min = delta;
//				i_delta_min = i;
//			}
//
//			// Next sample
//			pcam++;
//			i++;
//		}
//
//		// Compute position
//
//		position_temp = ((i_delta_max + i_delta_min)/2) - 65;
//
//		if ((position_temp <20) & (position_temp >-20))
//		{
//			g_position = -position_temp;
//		}
//
//		// Compute steering
//		g_steering = STEERING_KP * g_position;


		// Update Steering position
		TPM1->CONTROLS[0].CnV = g_steering_trim + g_steering;


		// 50Hz update
		vTaskDelay(20);
	}
}

*/
