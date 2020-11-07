/*
 * display.c
 *
 *  Created on: 9 mars 2019
 *      Author: Laurent
 */

#include "display.h"

const uint8_t	plut[64][2] = 	{ 	{7,0x80},{7,0x40},{7,0x20},{7,0x10},{7,0x08},{7,0x04},{7,0x02},{7,0x01},
		{6,0x80},{6,0x40},{6,0x20},{6,0x10},{6,0x08},{6,0x04},{6,0x02},{6,0x01},
		{5,0x80},{5,0x40},{5,0x20},{5,0x10},{5,0x08},{5,0x04},{5,0x02},{5,0x01},
		{4,0x80},{4,0x40},{4,0x20},{4,0x10},{4,0x08},{4,0x04},{4,0x02},{4,0x01},
		{3,0x80},{3,0x40},{3,0x20},{3,0x10},{3,0x08},{3,0x04},{3,0x02},{3,0x01},
		{2,0x80},{2,0x40},{2,0x20},{2,0x10},{2,0x08},{2,0x04},{2,0x02},{2,0x01},
		{1,0x80},{1,0x40},{1,0x20},{1,0x10},{1,0x08},{1,0x04},{1,0x02},{1,0x01},
		{0,0x80},{0,0x40},{0,0x20},{0,0x10},{0,0x08},{0,0x04},{0,0x02},{0,0x01}
};

xTaskHandle			vTaskDISP_handle;
xSemaphoreHandle	xDISPSem;

uint8_t				waveform[128];

volatile uint8_t	tx_buffer[21] = { 0 };





void vTaskDISP (void *pvParameters)
{
	tx_buffer[5] = 1;
	tx_buffer[6] = 2;


	uint8_t			sample, col, row[128];
	uint8_t			buffer[24];

	// Start with a fresh display
	BSP_OLED_Clear();

	// Initialize the row array (used to clear previous waveform before refresh)
	for (col=0; col<128; col++) row[col] = 0x00;

	// Main Loop
	while(1)
	{
		// Display camera data as 128-sample waveform
		for (col=0; col<128; col++)
		{
			sample = waveform[col];

			// Clear current row, only if different from previous one
			if (row[col] != plut[sample][0])
			{
				BSP_OLED_SendCommand( 0xB0 + row[col] );
				BSP_OLED_SendCommand( 0x00 + (col & 0x0F) );       	//set low col address
				BSP_OLED_SendCommand( 0x10 + ((col>>4) & 0x0F) );  	//set high col address

				BSP_OLED_SendData(0x00);
			}

			// Display new sample
			BSP_OLED_SendCommand( 0xB0 + plut[sample][0] );
			BSP_OLED_SendCommand( 0x00 + (col & 0x0F) );       		//set low col address
			BSP_OLED_SendCommand( 0x10 + ((col>>4) & 0x0F) );  		//set high col address

			BSP_OLED_SendData(plut[sample][1]);

			// Store current row for next time
			row[col] = plut[sample][0];
		}

		// Display Current Steering position
		my_sprintf((char *)buffer, "%04d", g_steering);
		BSP_OLED_setXY(0, 0);
		BSP_OLED_SendStr(buffer);

		my_sprintf((char *)buffer, "%04d", g_steering_trim);
		BSP_OLED_setXY(1, 0);
		BSP_OLED_SendStr(buffer);

		my_sprintf((char *)buffer, "%04d", g_adc_key);
		BSP_OLED_setXY(0, 10);
		BSP_OLED_SendStr(buffer);

		my_sprintf((char *)buffer, "%04d", g_speed_com_sp);
		BSP_OLED_setXY(1, 10);
		BSP_OLED_SendStr(buffer);

		my_sprintf((char *)buffer, "%3d", g_cam_expo);
		BSP_OLED_setXY(0, 5);
		BSP_OLED_SendStr(buffer);

		my_sprintf((char *)buffer, "%3d", g_position);
		BSP_OLED_setXY(1, 5);
		BSP_OLED_SendStr(buffer);



		// Test key value

		// 1023 -> No key
		if (g_adc_key > 1000)
		{
		}
/*
		// 830-840 -> Key_1
		if ((g_adc_key <840) && (g_adc_key > 830))
		{
			if (g_speed_com_sp > 0) g_speed_com_sp -= 10;
		}

		// 805-815 -> Key_2
		if ((g_adc_key <815) && (g_adc_key > 805))
		{
			if (g_speed_com_sp <= 900) g_speed_com_sp += 10;
		}
*/
		// 695-705 -> Key_3
		if ((g_adc_key <705) && (g_adc_key > 695))
		{
			if (g_cam_expo > 140) g_cam_expo -= 10;
		}

//		// 000-010 -> Key_4
		if (g_adc_key <010)
		{
			if (g_cam_expo < 480) g_cam_expo += 10;
		}

		// 600-610 -> Key_5
		if ((g_adc_key <610) && (g_adc_key > 600))
		{

		}

		//		tx_buffer[0]= g_error_steering;
		//		tx_buffer[1] = ' ';
		//		tx_buffer[2]= g_lineR;
		//		tx_buffer[3] = ' ';
		//		tx_buffer[4]= g_lineL;
		//		tx_buffer[5] = ' ';
		//		tx_buffer[6]= g_speed_sp[0];
		//		tx_buffer[7] = ' ';
		//		tx_buffer[8]= g_speed_sp[1];
		//		tx_buffer[9] = ' ';
		//		tx_buffer[10] = g_speed[0];
		//		tx_buffer[11] = ' ';
		//		tx_buffer[12] = g_speed[1];

		//my_sprintf(tx_buffer, "* %3d %3d %3d %3d %3d %3d %3d #", g_error_steering, g_lineR, g_lineL, g_speed_sp[0], g_speed_sp[1], g_speed[0], g_speed[1]);
		//my_sprintf(tx_buffer, "* %03d %03d #", g_error_steering, g_lineR);
		//Bit de poid fort puis faible

		tx_buffer[0] = '*';
		tx_buffer[1] = 21;
		tx_buffer[2] = g_speed_sp[0] >> 24;
		tx_buffer[3] = g_speed_sp[0] >> 16;
		tx_buffer[4] = g_speed_sp[0] >> 8;
		tx_buffer[5] = (uint8_t) g_speed_sp[0];

		tx_buffer[6] = g_speed[0] >> 24;
		tx_buffer[7] = g_speed[0] >> 16;
		tx_buffer[8] = g_speed[0] >> 8;
		tx_buffer[9] = (uint8_t) g_speed[0];

		tx_buffer[10] = g_speed_sp[1] >> 24;
		tx_buffer[11] = g_speed_sp[1] >> 16;
		tx_buffer[12] = g_speed_sp[1] >> 8;
		tx_buffer[13] = (uint8_t) g_speed_sp[1];

		tx_buffer[14] = g_speed[1] >> 24;
		tx_buffer[15] = g_speed[1] >> 16;
		tx_buffer[16] = g_speed[1] >> 8;
		tx_buffer[17] = (uint8_t) g_speed[1];

		tx_buffer[18] = (uint8_t) g_error_steering;

		tx_buffer[19] = (uint8_t)g_lineR;

		tx_buffer[20] = (uint8_t)g_lineL;



		//		tx_buffer[0] = '*';
		//		tx_buffer[1] = 21;
		//		tx_buffer[2] = g_speed_sp[0] >> 24;
		//		tx_buffer[3] = g_speed_sp[0] >> 16;
		//		tx_buffer[4] = g_speed_sp[0] >> 8;
		//		tx_buffer[5] = (uint8_t) g_speed_sp[0];
		//
		//		tx_buffer[6] = g_speed[0] >> 24;
		//		tx_buffer[7] = g_speed[0] >> 16;
		//		tx_buffer[8] = g_speed[0] >> 8;
		//		tx_buffer[9] = (uint8_t) g_speed[0];
		//
		//		tx_buffer[10] = g_speed_sp[1] >> 24;
		//		tx_buffer[11] = g_speed_sp[1] >> 16;
		//		tx_buffer[12] = g_speed_sp[1] >> 8;
		//		tx_buffer[13] = (uint8_t) g_speed_sp[1];
		//
		//		tx_buffer[14] = g_speed[1] >> 24;
		//		tx_buffer[15] = g_speed[1] >> 16;
		//		tx_buffer[16] = g_speed[1] >> 8;
		//		tx_buffer[17] = (uint8_t) g_speed[1];
		//
		//		tx_buffer[18] = (uint8_t) g_error_steering;
		//
		//		tx_buffer[19] = (uint8_t)g_lineR;
		//
		//		tx_buffer[20] = (uint8_t)g_lineL;




		//		my_sprintf(tx_buffer, "%c", " ");
		//		my_sprintf(tx_buffer, "%3d", g_error_steering);
		//		my_sprintf(tx_buffer, "%3d", g_error_steering);
		//		my_sprintf(tx_buffer, "%3d", g_error_steering);
		//		my_sprintf(tx_buffer, "%3d", g_error_steering);
		//		my_sprintf(tx_buffer, "%3d", g_error_steering);


		// Send data over Bluetooth with DMA
		DMA0->DMA[1].DSR_BCR |= DMA_DSR_BCR_DONE_MASK;	// Clear DONE flag
		DMA0->DMA[1].SAR = (uint32_t)&tx_buffer[0];		// Reload source address
		DMA0->DMA[1].DSR_BCR = DMA_DSR_BCR_BCR(21);		// Reload number of bytes to transfer
		DMA0->DMA[1].DCR |=    DMA_DCR_ERQ_MASK;		// Enable peripheral request

		// Give semaphore for waveform update request
		xSemaphoreGive(xDISPSem);

		// Wait 100ms
		vTaskDelay(100);
	}
}
