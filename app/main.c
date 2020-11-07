 
/**
 * @file    template4.c
 * @brief   Application entry point.
 */


// Headers
#include "main.h"
#include "splash.h"




// Global Variables and definitions

uint16_t	g_adc_key,
			g_lineR,
			g_lineL;

int16_t		g_position;

uint16_t	g_steering_trim;		// Used to set zero streering PWM
int16_t		g_steering;				// Steering command

int32_t		g_speed_com_sp;		// Common speed set point

int8_t		g_error_steering,
			valWhite;

int32_t		g_speed_sp[2],			// Speed set point 			(L/R)
			g_speed[2],				// Actual speed 			(L/R)
			g_speed_err[2],			// Speed error				(L/R)
			g_speed_int[2],			// Speed integrated error	(L/R)
			g_speed_cmd[2],			// Speed command			(L/R)
			g_delta,                // Delta error
			g_error_n,
			g_steering_kp;          //PI corrector value

uint16_t	g_cam_expo;				// Camera exposition
uint16_t*	g_pcam_buffer;			// Pointer to valid camera buffer



// Local functions
static void SystemClock_Config		(void);

// Local Tasks
void vTaskHWM (void *pvParameters);
xTaskHandle 	vTaskHWM_handle;





/*
 * @brief   Application entry point.
 */
int main(void)
{
	// Configure System Clock for 48MHz operation

 	SystemClock_Config();
	SystemCoreClockUpdate();

	uint8_t a = 0;
	// Initialize debug Console @115200 bauds
	BSP_Console_Init();
	my_printf("Console Ready!\r\n");

	// Initialize Bluetooth console
	BSP_BT_Console_Init();

	// Initialize Debug pins
	BSP_DBG_Pin_Init();

	// Initialize RGB LED pins
	BSP_LED_Pin_Init();

	// Start Steering PWM on PA12
	BSP_TPM1_PWM_Init();

	// Start Motor control PWM on PA4, PA5, PC8, PC9
	BSP_TPM0_PWM_Init();

	// Start speed encoder timers
	BSP_LPTMR0_Init();				// Right wheel
	BSP_TPM2_Init();				// Left  wheel

	// Initialize OLED SPI and pins
	BSP_OLED_SPI_Init();
	BSP_OLED_Reset();
	BSP_OLED_Init();

	delay_ms(100);

	BSP_OLED_SendBMP((uint8_t*)splash);
	delay_ms(100);

	// Start Trace Recording
	// my_printf("Awaiting Host to start Trace facility...");
	// vTraceEnable(TRC_START_AWAIT_HOST);
	vTraceEnable(TRC_START);
	///my_printf(" OK\r\n");

	// Create Semaphore to handle ACCL interrupt
	xACCLSem = xSemaphoreCreateBinary();
	vTraceSetSemaphoreName(xACCLSem, "ACCL");
	delay_ms(10);

	// Create Semaphore to handle CAM (DMA/ADC) interrupt
	xCAMSem = xSemaphoreCreateBinary();
	vTraceSetSemaphoreName(xCAMSem, "CAM");
	delay_ms(10);

	// Create Semaphore to handle display update
	xDISPSem = xSemaphoreCreateBinary();
	vTraceSetSemaphoreName(xDISPSem, "DISP");
	delay_ms(10);

	// Create Tasks
	xTaskCreate( vTaskHWM,  "Task_HWM"  , 160, NULL, 1, &vTaskHWM_handle  );
	delay_ms(10);
	xTaskCreate( vTaskDISP, "Task_DISP" , 160, NULL, 2, &vTaskDISP_handle );
	delay_ms(10);
	xTaskCreate( vTaskACCL, "Task_ACCL" , 128, NULL, 3, &vTaskACCL_handle );
	delay_ms(10);
	xTaskCreate( vTaskCAM,  "Task_CAM"  , 256, NULL, 4, &vTaskCAM_handle  );
	delay_ms(10);
	xTaskCreate( vTaskCTRL, "Task_CTRL" , 384, NULL, 5, &vTaskCTRL_handle );
	delay_ms(10);

	// Start the Scheduler
	vTaskStartScheduler();

	while(1)
	{
			// The program should never be here...
	}
}


/*
 * vTaskHWM
 */
void vTaskHWM (void *pvParameters)
{
	uint32_t	count;
	uint16_t	hwm_TaskCTRL, hwm_TaskCAM, hwm_TaskACCL, hwm_TaskDISP, hwm_TaskHWM;
	uint32_t	free_heap_size;

	count = 0;

	// Allow some time for other tasks to start (and use console)
	vTaskDelay(1000);

	// Prepare console layout using ANSI escape sequences
	my_printf("%c[0m",   0x1B);		// Remove all text attributes
	my_printf("%c[2J",   0x1B); 	// Clear console
	my_printf("%c[1;0H", 0x1B);		// Move cursor [1:0]

	my_printf("High Water Marks console");

	my_printf("%c[3;0H", 0x1B);	// Move cursor line 3
	my_printf("Iteration");

	my_printf("%c[4;0H", 0x1B);	// Move cursor line 4
	my_printf("Task CTRL");

	my_printf("%c[5;0H", 0x1B);	// Move cursor line 5
	my_printf("Task CAM");

	my_printf("%c[6;0H", 0x1B);	// Move cursor line 6
	my_printf("Task ACCL");

	my_printf("%c[7;0H", 0x1B);	// Move cursor line 6
	my_printf("Task DISP");

	my_printf("%c[8;0H", 0x1B);	// Move cursor line 6
	my_printf("Task HWM");

	my_printf("%c[9;0H", 0x1B);	// Move cursor line 7
	my_printf("Free Heap");


	while(1)
	{
	  // Gather High Water Marks
	  hwm_TaskCTRL	= 	uxTaskGetStackHighWaterMark(vTaskCTRL_handle);
	  hwm_TaskCAM 	= 	uxTaskGetStackHighWaterMark(vTaskCAM_handle);
	  hwm_TaskACCL	= 	uxTaskGetStackHighWaterMark(vTaskACCL_handle);
	  hwm_TaskDISP  =   uxTaskGetStackHighWaterMark(vTaskDISP_handle);
	  hwm_TaskHWM	= 	 uxTaskGetStackHighWaterMark(vTaskHWM_handle);

	  // Get free Heap size
	  free_heap_size = xPortGetFreeHeapSize();

	  // Display results into console
	  my_printf("%c[0;31;40m", 0x1B); 	// Red over black

	  my_printf("%c[3;12H", 0x1B);
	  my_printf("%5d", count);

	  my_printf("%c[1;33;44m", 0x1B); 	// Yellow over blue

	  my_printf("%c[4;12H", 0x1B);
	  my_printf("%5d", hwm_TaskCTRL);

	  my_printf("%c[5;12H", 0x1B);
	  my_printf("%5d", hwm_TaskCAM);

	  my_printf("%c[6;12H", 0x1B);
	  my_printf("%5d", hwm_TaskACCL);

	  my_printf("%c[7;12H", 0x1B);
	  my_printf("%5d", hwm_TaskDISP);

	  my_printf("%c[8;12H", 0x1B);
	  my_printf("%5d", hwm_TaskHWM);

	  my_printf("%c[1;35;40m", 0x1B); 	// Majenta over black
	  my_printf("%c[9;12H", 0x1B);
	  my_printf("%5d", free_heap_size);

	  my_printf("%c[0m", 0x1B); 		// Remove all text attributes
	  count++;

	  // Wait for 200ms
	  vTaskDelay(200);
	}
}


/*
 * SystemClock_Config()
 *
 * Clock setup for 48MHz operation from 8MHz external clock
 */

static void SystemClock_Config()
{
	// Safe clock division
	SIM->CLKDIV1 = 0x10030000U;

	// Enable external clock with no capacitor to drive
	OSC0->CR = 0x80;

	// Select External Clock with High-Frequency Range
	MCG->C2 &= ~MCG_C2_RANGE0_MASK;
	MCG->C2 |= MCG_C2_RANGE(1U) | 0x04;

	// Wait for stable oscillator
	while (!(MCG->S & MCG_S_OSCINIT0_MASK));

	// Set FLL division to /1
	MCG->C1 = ((MCG->C1 & ~MCG_C1_FRDIV_MASK) | MCG_C1_FRDIV(0));

	// Disable Low-Power
	MCG->C2 &= ~MCG_C2_LP_MASK;

	// Change to use external clock first
	MCG->C1 = ((MCG->C1 & ~(MCG_C1_CLKS_MASK | MCG_C1_IREFS_MASK)) | MCG_C1_CLKS(2U));

	// Wait for CLKST clock status bits
	while ((MCG->S & (MCG_S_IREFST_MASK | MCG_S_CLKST_MASK)) != (MCG_S_IREFST(0U) | MCG_S_CLKST(2U)));

	// Disable PLL first, then configure PLL
	MCG->C6 &= ~MCG_C6_PLLS_MASK;
	while (MCG->S & MCG_S_PLLST_MASK);

	// Set prediv to /2 to achieve 4MHz at PLL input
	MCG->C5 = MCG_C5_PRDIV0(1U);

	// Set VDIV (M) to x24 -> 96MHz
	MCG->C6 = (MCG->C6 & ~MCG_C6_VDIV0_MASK) | MCG_C6_VDIV0(0U);

	// Set enable mode
	MCG->C5 |= ((uint32_t)0x40);

	// Wait for PLL lock
	while (!(MCG->S & MCG_S_LOCK0_MASK));

	// Change to PLL mode
	MCG->C6 |= MCG_C6_PLLS_MASK;

	// Wait for PLL mode changed
	while (!(MCG->S & MCG_S_PLLST_MASK));

	// Change to use PLL output clock
	MCG->C1 = (MCG->C1 & ~MCG_C1_CLKS_MASK) | MCG_C1_CLKS(0U);
	while ( (MCG->S & MCG_S_CLKST_MASK) != MCG_S_CLKST(3U) );

	// Update FCRDIV
	MCG->SC = (MCG->SC & ~(MCG_SC_FCRDIV_MASK | MCG_SC_ATMF_MASK | MCG_SC_LOCS0_MASK)) | MCG_SC_FCRDIV(0U);

	// Set internal reference clock selection to slow clock
	MCG->C2 = (MCG->C2 & ~MCG_C2_IRCS_MASK) | (MCG_C2_IRCS(0U));
	MCG->C1 = (MCG->C1 & ~(MCG_C1_IRCLKEN_MASK | MCG_C1_IREFSTEN_MASK)) | (uint8_t)2U;

	// Wait for slow clock 32kHz ready
	while ( (MCG->S & MCG_S_IRCST_MASK) != MCG_S_IRCST(0U) );

	// Set DIV1 and DIV4 to /2 -> 48MHz (CPU), 24MHz (Buses)
	SIM->CLKDIV1 = 0x10010000;

	// Fixed division /2 for other peripheral
	SIM->SOPT2 = ((SIM->SOPT2 & ~SIM_SOPT2_PLLFLLSEL_MASK) | SIM_SOPT2_PLLFLLSEL(1U));

	// Set the OSC32 source to LPO 1kHz
	SIM->SOPT1 = ((SIM->SOPT1 & ~SIM_SOPT1_OSC32KSEL_MASK) | SIM_SOPT1_OSC32KSEL(3U));

	// Setup PC3 as CLKOUT

	// Start GPIOC clock
	SIM->SCGC5 |= SIM_SCGC5_PORTC(1U);

	GPIOC->PDDR |= 1U <<3U;				// Set PC3 as output

	// Connect PC3  to AF5 (CLKOUT)
	PORTC->PCR[3] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[3] |= PORT_PCR_MUX(5U);

	// CLKOUT is MCGIRCLK
	SIM->SOPT2 &= ~SIM_SOPT2_CLKOUTSEL_MASK;
	SIM->SOPT2 |= SIM_SOPT2_CLKOUTSEL(4);
}







