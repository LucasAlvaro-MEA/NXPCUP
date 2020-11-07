/*
 * cam.c
 *
 *  Created on: 8 mars 2019
 *      Author: Laurent
 */

#include "cam.h"

// Global variables

// volatile uint16_t  __attribute__ ((aligned (32))) tls_buffer_A[EXPO_NCLK];	// ADC DMA buffers A/B
// volatile uint16_t  __attribute__ ((aligned (32))) tls_buffer_B[EXPO_NCLK];

volatile uint16_t  __attribute__ ((aligned (32))) tls_buffer_A[512];	// ADC DMA buffers A/B
volatile uint16_t  __attribute__ ((aligned (32))) tls_buffer_B[512];

//ZAZA

static uint8_t exposition_initCheck(void);
static uint8_t findBlack(void);
//static void detecteLigne(void);
//ZAZA end


uint8_t				tls_buffer_id;											// ADC DMA buffers ID

xTaskHandle			vTaskCAM_handle;
xSemaphoreHandle	xCAMSem;


void vTaskCAM (void *pvParameters)
{
	uint8_t			i;

	portBASE_TYPE	xStatus;

	uint16_t		*current_buffer;
	//ZAZA var
	uint8_t		camIsInit=0, tour=0, startInit=0, whiteOk = 0;

	my_printf("Entering Task CAM\r\n");

	g_cam_expo = EXPO_NCLK;

	// Initialize TSL1401 (Optical sensor) pins
	BSP_TSL1401_Pins_Init();

	// Initialize ADC0 DMA process
	BSP_ADC_DMA_Init();

	// Take the semaphore once to make sure it is empty
	xSemaphoreTake(xCAMSem, 0);

	// Start with tls_buffer_A
	tls_buffer_id = 1;

	// Enable DMA0 interrupt
	NVIC_SetPriority(DMA0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+0);
	NVIC_EnableIRQ(DMA0_IRQn);

	// Start ADC0 conversion on Channel 11 without interrupt
	ADC0->SC1[0] = ADC_SC1_ADCH(11U);

	while(1)
	{
		GPIOC->PCOR =  1U <<12U;				// Set Output C12 low

		// Wait for the DMA/ADC interrupt semaphore
		xSemaphoreTake(xCAMSem, portMAX_DELAY);

		// Send SI pulse on C4 pin
		GPIOC->PSOR =  1U <<4U;					// Drive SI  high

		// Start KEY value conversion on PC0 (Channel 14)
		ADC0->SC1[0] = ADC_SC1_ADCH(14U);

		// Wait until ADC result is available
		while((ADC0->SC1[0] & ADC_SC1_COCO_MASK) == 0);

		// Read result
		g_adc_key = ADC0->R[0];

		// Stop ADC0 conversion
		ADC0->SC1[0] = ADC_SC1_ADCH(0x1F);

		// delay_us(30);						// Allow some time for clock to rise a couple of time
		GPIOC->PCOR =  1U <<4U;					// Drive SI  low

		// Re-Set DMA0
		switch (tls_buffer_id)
		{
			case 1:
			{
				current_buffer = (uint16_t *)&tls_buffer_A[0];
				DMA0->DMA[0].DAR = (uint32_t)&tls_buffer_B[0];				// Current Destination

				tls_buffer_id = 2;											// ... next time
				break;
			}

			case 2:
			{
				current_buffer = (uint16_t *)&tls_buffer_B[0];
				DMA0->DMA[0].DAR = (uint32_t)&tls_buffer_A[0];				// Current Destination

				tls_buffer_id = 1;											// ... next time
				break;
			}
		}

		DMA0->DMA[0].DSR_BCR |= DMA_DSR_BCR_BCR(g_cam_expo*2);		// Set byte count register
		DMA0->DMA[0].DCR 	 |= DMA_DCR_ERQ_MASK;					// Re-Enable peripheral request

		// Read ADC data to clear any pending requests
		ADC0->R[0];

		// Enable ADC DMA request
		ADC0->SC2 |= ADC_SC2_DMAEN(1U);

		// Enable DMA channel and source
		DMAMUX0->CHCFG[0] |= DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_SOURCE(40);

		// Start ADC0 conversion on Channel 11 without interrupt
		ADC0->SC1[0] = ADC_SC1_ADCH(11U);

		// If display needs update (continue otherwise)
		xStatus = xSemaphoreTake(xDISPSem, 0);

		if (xStatus == pdPASS)
		{
			for (i=0; i<128; i++)
			{
				waveform[i] = (uint8_t)(current_buffer[i] >>4U);
			}
		}
		//ZAZA
		//Init the luminosity while it's not enough good
		if(!camIsInit && tour>20 && startInit==200)
			{
			camIsInit=exposition_initCheck();
			my_printf("camisinit %d\r\n", camIsInit);
			BSP_LED_On(0);

			tour = 0;
			}
		if (camIsInit && !whiteOk)
		{
			valWhite = findBlack();
			BSP_LED_On(2);
			whiteOk = 1;
			my_printf("derivative %d\r\n", whiteOk);
		}
//ZAZA end

		// Update global pointer to valid camera buffer
		g_pcam_buffer = current_buffer;
		tour++;
		if(startInit!=200) startInit++;

	}
}


/*
 * Handles DMA0 interrupt
 * Resets the BCR register and clears the DONE flag
 */

void DMA0_IRQHandler()
{
	portBASE_TYPE 	xHigherPriorityTaskWoken = pdFALSE;

	GPIOC->PSOR =  1U <<12U;				// Set Output C12 high

	// Disable ADC DMA request
	ADC0->SC2 &= ~ADC_SC2_DMAEN_MASK;

	// Disable DMA channel
	DMAMUX0->CHCFG[0] = 0x00;

	// Clear DONE flag
	DMA0->DMA[0].DSR_BCR |= DMA_DSR_BCR_DONE_MASK;

	// Stop ADC0 conversion
	ADC0->SC1[0] = ADC_SC1_ADCH(0x1F);

	// Release CAM semaphore
	xSemaphoreGiveFromISR(xCAMSem, &xHigherPriorityTaskWoken);

	// Perform a context switch to the waiting task
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

// ZAZA

uint8_t exposition_initCheck(){


	uint8_t			pcam[128];
	uint16_t colorAVG;

	//Take the camera buffer
	for (int i=0; i<128; i++)
		{
		pcam[i] = waveform[i];
		}

	colorAVG = (uint16_t)((pcam[62] + pcam[63] + pcam[64] + pcam[65])/4);
//	colorAVG = pcam[64];

	if ((colorAVG)<57 || (colorAVG)>59 && (g_cam_expo >= 140 && g_cam_expo <= 500))
	{
		if((colorAVG)<57)
		{
			if (g_cam_expo < 500) g_cam_expo+= 10;
			my_printf(" CAM INIT +10expo %d,case %d\r\n",g_cam_expo, colorAVG);
		}
		else
		{
			if (g_cam_expo > 140) g_cam_expo-= 9;
			my_printf("CAM INIT -9 expo %d,case %d\r\n",g_cam_expo, colorAVG);
		}
		return 0;
	}
	else return 1;

}

uint8_t findBlack()
/*
 * Cette fonction renvoie la valeur du blanc minimal sur le circuit.
 */
{

	uint8_t			pcam[128];

	//Take the camera buffer
	for (int i=0; i<128; i++)
		{
		pcam[i] = waveform[i];
		}
	int8_t diffMax = 0;
	uint8_t indiceMax = 64;
	uint8_t valWhite = 0;
	// On regarde la différence max entre les 64 pixels de gauche de la cam
	for (int i = 64;i> 0; i--)
	{
		if(pcam[i + 1] - pcam[i] > diffMax)
		{
			diffMax = pcam[i + 1] - pcam[i];
			indiceMax = i;
		}
	}
	valWhite = pcam[indiceMax] + pcam[indiceMax]/10 + 1;
	if (valWhite < 10)
	{
		valWhite = 0;
	}
		return valWhite;

}
