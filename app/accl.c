/*
 * accl.c
 *
 *  Created on: 9 mars 2019
 *      Author: Laurent
 */

#include "accl.h"

// Global variables

xTaskHandle			vTaskACCL_handle;
xSemaphoreHandle	xACCLSem;
int16_t				accl_x, accl_y, accl_z;


void vTaskACCL (void *pvParameters)
{
	uint8_t		mma_id;
	uint8_t		accl_data[6];

	my_printf("Entering Task ACCL\r\n");

	// Initialize MMA8451 I2C and INT pin
	MMA8451_I2C_Init();
	MMA8451_INT_Pin_Init();
	delay_ms(10);

	// Read MMA8451 ID
	mma_id = MMA8451_ReadByte(WHO_AM_I_REG);
	my_printf("MMA8451 ID = 0x%02x\r\n", mma_id);

	// Initialize MMA8451
	MMA8451_Init();
	delay_ms(10);

	// Take the semaphore once to make sure it is empty
	xSemaphoreTake(xACCLSem, 0);

	// Enable PORTA Interrupt (MMA8451 INT pin on PA14)
	NVIC_SetPriority(PORTA_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+2);
	NVIC_EnableIRQ(PORTA_IRQn);


	my_printf("Entering Task ACCL loop\r\n");

	while(1)
	{
		// Wait for the IMU interrupt semaphore
		xSemaphoreTake(xACCLSem, portMAX_DELAY);

		// Debug
		// while( (UART0->S1 & UART0_S1_TDRE_MASK) == 0 );
		// UART0->D = '*';

		// Read data from Accelerometer
		MMA8451_ReadNBytes(OUT_X_MSB_REG, 6, accl_data);

		// Compute 3-axis acceleration as 14-bits signed integers
		accl_x = ((int16_t) (accl_data[0]<<8 | accl_data[1])) >> 2;		// Compute 14-bit X-axis output value
		accl_y = ((int16_t) (accl_data[2]<<8 | accl_data[3])) >> 2;		// Compute 14-bit Y-axis output value
		accl_z = ((int16_t) (accl_data[4]<<8 | accl_data[5])) >> 2;		// Compute 14-bit Z-axis output value
	}
}




/*
 * PORTA_IRQHandler()
 *
 * Handles PA14 interruptions (MMA8451 Accelerometer DATA READY event)
 *
 */

void PORTA_IRQHandler()
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	if ((PORTA->PCR[14] & PORT_PCR_ISF_MASK) != 0)
	{
		// Clear Interrupt flag
		PORTA->PCR[14] |= PORT_PCR_ISF(1U);

		// Release ACCL semaphore
		xSemaphoreGiveFromISR(xACCLSem, &xHigherPriorityTaskWoken);

		// Perform a context switch to the waiting task
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}
}
