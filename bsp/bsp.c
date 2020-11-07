/*
 * bsp.c
 *
 *  Created on: 23 févr. 2019
 *      Author: Laurent
 */


#include "MKL25Z4.h"
#include "main.h"
#include "bsp.h"

void BSP_LED_Pin_Init()
{
	// Setup LED pins (B18, B19, D1)

	SIM->SCGC5 |= SIM_SCGC5_PORTB(1U);		// Start GPIOB clock
	SIM->SCGC5 |= SIM_SCGC5_PORTD(1U);		// Start GPIOD clock

	PORTB->PCR[18] &= ~PORT_PCR_MUX_MASK;	// Connect B18 pin to AF1 (GPIO)
	PORTB->PCR[18] |= PORT_PCR_MUX(1U);

	PORTB->PCR[19] &= ~PORT_PCR_MUX_MASK;	// Connect B19 pin to AF1 (GPIO)
	PORTB->PCR[19] |= PORT_PCR_MUX(1U);

	PORTD->PCR[1] &= ~PORT_PCR_MUX_MASK;	// Connect D1 pin to AF1 (GPIO)
	PORTD->PCR[1] |= PORT_PCR_MUX(1U);

	GPIOB->PSOR =  1U <<18U;				// Set Output B18
	GPIOB->PSOR =  1U <<19U;				// Set Output B19
	GPIOD->PSOR =  1U <<1U;					// Set Output D1

	GPIOB->PDDR |= 1U <<18U;				// Set B18 as output
	GPIOB->PDDR |= 1U <<19U;				// Set B19 as output
	GPIOD->PDDR |= 1U <<1U;					// Set D1  as output
}


void BSP_LED_On(uint8_t color)
{
	switch(color)
	{
		case 0:		// RED
		{
			GPIOB->PCOR =  1U <<18U;				// Red   in on
			GPIOB->PSOR =  1U <<19U;				// Green is off
			GPIOD->PSOR =  1U <<1U;					// Blue  is off
			break;
		}

		case 1:		// YELLOW
		{
			GPIOB->PCOR =  1U <<18U;				// Red   in on
			GPIOB->PCOR =  1U <<19U;				// Green is on
			GPIOD->PSOR =  1U <<1U;					// Blue  is off
			break;
		}

		case 2:		// GREEN
		{
			GPIOB->PSOR =  1U <<18U;				// Red   in off
			GPIOB->PCOR =  1U <<19U;				// Green is on
			GPIOD->PSOR =  1U <<1U;					// Blue  is off
			break;
		}

		case 3:		//	CYAN
		{
			GPIOB->PSOR =  1U <<18U;				// Red   in off
			GPIOB->PCOR =  1U <<19U;				// Green is on
			GPIOD->PCOR =  1U <<1U;					// Blue  is on
			break;
		}

		case 4:		// BLUE
		{
			GPIOB->PSOR =  1U <<18U;				// Red   in off
			GPIOB->PSOR =  1U <<19U;				// Green is off
			GPIOD->PCOR =  1U <<1U;					// Blue  is on
			break;
		}

		case 5:		// MAGENTA
		{
			GPIOB->PCOR =  1U <<18U;				// Red   in on
			GPIOB->PSOR =  1U <<19U;				// Green is off
			GPIOD->PCOR =  1U <<1U;					// Blue  is on
			break;
		}

		case 6:		// WHITE
		{
			GPIOB->PCOR =  1U <<18U;				// Red   in on
			GPIOB->PCOR =  1U <<19U;				// Green is on
			GPIOD->PCOR =  1U <<1U;					// Blue  is on
			break;
		}
	}
}


void BSP_LED_Off()
{
	GPIOB->PSOR =  1U <<18U;				// Red   in off
	GPIOB->PSOR =  1U <<19U;				// Green is off
	GPIOD->PSOR =  1U <<1U;					// Blue  is off
}



void BSP_DBG_Pin_Init()
{
	// Setup Debug pins (C12, C13)

	SIM->SCGC5 |= SIM_SCGC5_PORTC(1U);		// Start GPIOC clock

	PORTC->PCR[12] &= ~PORT_PCR_MUX_MASK;	// Connect C12 pin to AF1 (GPIO)
	PORTC->PCR[12] |= PORT_PCR_MUX(1U);

	PORTC->PCR[13] &= ~PORT_PCR_MUX_MASK;	// Connect C13 pin to AF1 (GPIO)
	PORTC->PCR[13] |= PORT_PCR_MUX(1U);

	GPIOC->PCOR =  1U <<12U;				// Set Output C12 low
	GPIOC->PCOR =  1U <<13U;				// Set Output C13 low

	GPIOC->PDDR |= 1U <<12U;				// Set C12 as output
	GPIOC->PDDR |= 1U <<13U;				// Set C13 as output
}


void BSP_Console_Init()
{
	// Start GPIOA clock
	SIM->SCGC5 |= SIM_SCGC5_PORTA(1U);

	PORTA->PCR[1] &= ~PORT_PCR_MUX_MASK;	// Connect A1 pin to AF2 (UART0_RX)
	PORTA->PCR[1] |= PORT_PCR_MUX(2U);

	PORTA->PCR[2] &= ~PORT_PCR_MUX_MASK;	// Connect A2 pin to AF2 (UART0_TX)
	PORTA->PCR[2] |= PORT_PCR_MUX(2U);

	// UART0 TX/RX data source select TX/RX pins
	SIM->SOPT5 &= ~( SIM_SOPT5_UART0TXSRC_MASK | SIM_SOPT5_UART0RXSRC_MASK );
	// SIM->SOPT5 !=  ( SIM_SOPT5_UART0TXSRC(0U) | SIM_SOPT5_UART0RXSRC(0U) );

	// Start UART0 clock
	SIM->SCGC4 |= SIM_SCGC4_UART0(1U);

	// Clock source is MCGPLLCLK/2 = 48MHz
	SIM->SOPT2 = ((SIM->SOPT2 & ~SIM_SOPT2_UART0SRC_MASK) | SIM_SOPT2_UART0SRC(1U));

	// 48Mhz / (32 * 13) = 115384.61bps -> 0.16% error
	// Set OSR value to /32
	UART0->C4 = ((UART0->C4 & ~UART0_C4_OSR_MASK) | (32 - 1));

	// program the divider /13
	UART0->BDH = (uint8_t)0x00;
	UART0->BDL = (uint8_t)0x0D;

	// Default configuration
	UART0->C1 = 0x00;
	UART0->C2 = 0x00;

	// Enable TX & RX
	UART0->C2 |= UART0_C2_TE(1U) | UART0_C2_RE(1U);
}



extern volatile uint8_t  tx_buffer[12];

void BSP_BT_Console_Init()
{
	// Start GPIOE clock
	SIM->SCGC5 |= SIM_SCGC5_PORTE(1U);

	PORTE->PCR[22] &= ~PORT_PCR_MUX_MASK;	// Connect E22 pin to AF4 (UART2_TX)
	PORTE->PCR[22] |=  PORT_PCR_MUX(4U);

	PORTE->PCR[23] &= ~PORT_PCR_MUX_MASK;	// Connect E23 pin to AF4 (UART2_RX)
	PORTE->PCR[23] |=  PORT_PCR_MUX(4U);

	// Start UART2 clock
	SIM->SCGC4 |= SIM_SCGC4_UART2(1U);

	// Configure Baudrate = (48MHz/2) / (16 * BR)
	// @9600bps -> BR = 156 = 0x09C
	// @115200  -> BR = 13  = 0x00D
	UART2->BDH = 0x00;
	UART2->BDL = 0x0D;

	// Default configuration
	UART2->C1 = 0x00;
	UART2->C2 = 0x00;

	// Enable TX & RX
	UART2->C2 |= UART_C2_TE(1U) | UART_C2_RE(1U);


	// Setup DMA for TX

	// Enable DMA clock
	SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK;
	SIM->SCGC7 |= SIM_SCGC7_DMA_MASK;

	// Disable DMA Mux channel first
	DMAMUX0->CHCFG[1] = 0x00;

	// Configure DMA for UART2_TX operation
	DMA0->DMA[1].SAR = (uint32_t)&tx_buffer[0];		// Source
	DMA0->DMA[1].DAR = (uint32_t)&(UART2->D);		// Destination

	DMA0->DMA[1].DCR = 0x00000000;

	DMA0->DMA[1].DCR |= ( // DMA_DCR_EINT_MASK	|		// Enable interrupt
				 	 	  // DMA_DCR_ERQ_MASK	|		// Enable peripheral request
						  DMA_DCR_CS_MASK  		|		// Single transfer per UART request
						  // DMA_DCR_AA_MASK 	|		// Enable Auto-Align
						  DMA_DCR_D_REQ(1U)		|		// Disable peripheral request upon completion
						  DMA_DCR_SINC(1U) 		|		// Increment source address
						  DMA_DCR_SSIZE(1U)		|		// Set source size to 8 bits
						  // DMA_DCR_DINC_MASK	|		// Set increments to destination address
						  DMA_DCR_DSIZE(1U));			// Set destination size of 8 bits

	// Enable DMA channel and source
	// UART2_RX -> Source number 6
	// UART2_TX -> Source number 7

	DMAMUX0->CHCFG[1] |= DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_SOURCE(7);

	// Enable UART2_TX DMA request when Transmit Register is empty
	UART2->C4 |= UART_C4_TDMAS(1U);
	UART2->C2 |= UART_C2_TIE(1U);
}




void BSP_LPTMR0_Init()
{
	// Pulse counting on PTC5 (Right-Wheel)

	// Start GPIOC clock
	SIM->SCGC5 |= SIM_SCGC5_PORTC(1U);

	// Connect C5 pin to AF3 (LPTMR0_ALT2)
	PORTC->PCR[5] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[5] |= PORT_PCR_MUX(3U);

	// Make sure PTC5 is input
	GPIOC->PDDR &= ~(0x01 <<5U);

	// Select OSC32KCLK as Oscillator
	// SIM->SOPT1 &= ~SIM_SOPT1_OSC32KSEL_MASK;

	// Enable access to LPTMR0
	SIM->SCGC5 |= SIM_SCGC5_LPTMR(1U);

	// Reset configuration
	LPTMR0->CSR = 0x00000000;
	LPTMR0->PSR = 0x00000000;

	// Set Pulse_mode
	LPTMR0->CSR |= LPTMR_CSR_TMS(1U);

	// Select Input pin ALT2
	LPTMR0->CSR |= LPTMR_CSR_TPS(2U);

	// CNR reset on overflow only
	LPTMR0->CSR |= LPTMR_CSR_TFC(1U);

	// Bypass the Prescaler module
	LPTMR0->PSR |= LPTMR_PSR_PBYP(1U);

	// Select clock source
	// LPTMR0->PSR |= LPTMR_PSR_PCS(0U);

	// Start LPTMR0
	LPTMR0->CSR |= LPTMR_CSR_TEN(1U);
}


void BSP_TPM2_Init()
{
	// Pulse counting on PTE29 (TPM_CLKIN0, AF4)

	// Start GPIOE clock
	SIM->SCGC5 |= SIM_SCGC5_PORTE(1U);

	// Connect E29 pin to AF4 (TPM_CLKIN0)
	PORTE->PCR[29] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[29] |= PORT_PCR_MUX(4U);

	// Make sure PTE29 is input
	GPIOE->PDDR &= ~(0x01 <<29U);

	// Start TPM2 clock
	SIM->SCGC6 |= SIM_SCGC6_TPM2(1U);

	// Reset configuration
	TPM2->SC = 0x00000000;

	// Clock source is MCGPLLCLK/2 = 48MHz
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1U);

	// Set clock prescaler to /1 (external clock divider)
	TPM2->SC &= ~TPM_SC_PS_MASK;
	TPM2->SC |= TPM_SC_PS(0U);

	// Set CLKIN0 as TPM2 external clock source
	SIM->SOPT4 &= ~(SIM_SOPT4_TPM2CLKSEL_MASK);
	SIM->SOPT4 |=   SIM_SOPT4_TPM2CLKSEL(0U);

	// Keep counter running in debug mode
	TPM2->CONF |= TPM_CONF_DBGMODE(3U);

	// Set Period to maximum
	TPM2->MOD = 0xFFFF;

	// Start Counter with external clock
	TPM2->SC |= TPM_SC_CMOD(2U);
}


void BSP_TPM1_PWM_Init()
{
	// Steering PWM -> PTA12 (TPM1, Channel0)

	// Start GPIOA clock
	SIM->SCGC5 |= SIM_SCGC5_PORTA(1U);

	// Connect A12 pin to AF3 (TPM1, Channel 0)
	PORTA->PCR[12] &= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[12] |= PORT_PCR_MUX(3U);

	// Start TPM1 clock
	SIM->SCGC6 |= SIM_SCGC6_TPM1(1U);

	// Clock source is MCGPLLCLK/2 = 48MHz
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1U);

	// Keep counter running in debug mode
	TPM1->CONF |= TPM_CONF_DBGMODE(3U);

	// Set clock prescaler to /32 -> 1.5MHz counting frequency
	TPM1->SC &= ~TPM_SC_PS_MASK;
	TPM1->SC |= TPM_SC_PS(5U);

	// Set Channel 0 to Edge-Aligned PWM mode
	TPM1->CONTROLS[0].CnSC = 0x28;

	// Set Period
	TPM1->MOD = 16500; 				// for 11ms period

	// Set pulse width
	TPM1->CONTROLS[0].CnV = 2250;	// for 1.5ms width. Range is [1500 -> 3000]

	// Start Counter
	TPM1->SC |= TPM_SC_CMOD(1U);
}


void BSP_TPM0_PWM_Init()
{
	// Motor 1 PWM 1	-> PA4
	// Motor 1 PWM 2	-> PA5
	// Motor 2 PWM 1	-> PC8
	// Motor 2 PWM 2	-> PC9


	// Start GPIOA clock
	SIM->SCGC5 |= SIM_SCGC5_PORTA(1U);

	// Connect A4 & A5 pin to AF3 (TPM0, Channel 1&2)
	PORTA->PCR[4] &= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[4] |= PORT_PCR_MUX(3U);

	PORTA->PCR[5] &= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[5] |= PORT_PCR_MUX(3U);

	// Start GPIOC clock
	SIM->SCGC5 |= SIM_SCGC5_PORTC(1U);

	// Connect C8 & C9 pin to AF3 (TPM0, Channel 4&5)
	PORTC->PCR[8] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[8] |= PORT_PCR_MUX(3U);

	PORTC->PCR[9] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[9] |= PORT_PCR_MUX(3U);

	// Start TPM0 clock
	SIM->SCGC6 |= SIM_SCGC6_TPM0(1U);

	// Clock source is MCGPLLCLK/2 = 48MHz
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1U);

	// Keep counter running in debug mode
	TPM0->CONF |= TPM_CONF_DBGMODE(3U);

	// Set clock prescaler to /2 -> 24MHz counting frequency
	TPM0->SC &= ~TPM_SC_PS_MASK;
	TPM0->SC |= TPM_SC_PS(1U);

	// Set Channel 1,2,4,5 to Edge-Aligned PWM mode
	TPM0->CONTROLS[1].CnSC = 0x28;
	TPM0->CONTROLS[2].CnSC = 0x28;
	TPM0->CONTROLS[4].CnSC = 0x28;
	TPM0->CONTROLS[5].CnSC = 0x28;

	// Set Period
	TPM0->MOD = 1000; 				// 24kHz PWM frequency

	// Set duty-cycle
	TPM0->CONTROLS[1].CnV = 0;
	TPM0->CONTROLS[2].CnV = 0;
	TPM0->CONTROLS[4].CnV = 0;
	TPM0->CONTROLS[5].CnV = 0;

	// Start Counter
	TPM0->SC |= TPM_SC_CMOD(1U);
}



extern volatile uint16_t  tls_buffer_A[EXPO_NCLK];
extern volatile uint16_t  tls_buffer_B[EXPO_NCLK];


void BSP_ADC_DMA_Init()
{
	// CCD Analog output -> C2 (ADC0, channel 11)
	// Harware trigger 	 -> PC6 (EXTRG_IN pin, externally connected to PC3 CLKOUT pin)

	// Start GPIOC clock
	SIM->SCGC5 |= SIM_SCGC5_PORTC(1U);

	// Set PC6 (EXTRG_IN) as input
	GPIOC->PDDR &= ~(1U <<6U);

	// Connect PC6 to AF3 (EXTRG_IN)
	PORTC->PCR[6] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[6] |= PORT_PCR_MUX(3U);

	// Start ADC0 clock
	SIM->SCGC6 |= SIM_SCGC6_ADC0(1U);

	// Select Bus Clock /2 as clock source
	ADC0->CFG1 &= ~ADC_CFG1_ADICLK_MASK;
	ADC0->CFG1 |=  ADC_CFG1_ADICLK(1U);

	// Set resolution to 10-bit
	ADC0->CFG1 &= ~ADC_CFG1_MODE_MASK;
	ADC0->CFG1 |=  ADC_CFG1_MODE(2U);

	// Select EXTRG (External Trigger pin) for ADC
	SIM->SOPT7 &= ~(SIM_SOPT7_ADC0TRGSEL_MASK | SIM_SOPT7_ADC0PRETRGSEL_MASK | SIM_SOPT7_ADC0ALTTRGEN_MASK);
	SIM->SOPT7 |= SIM_SOPT7_ADC0TRGSEL(0U) | SIM_SOPT7_ADC0PRETRGSEL(0U) | SIM_SOPT7_ADC0ALTTRGEN(1U) ;

	// Set to single conversion mode
	ADC0->SC3 = 0x00;

	// Set to continuous conversion mode
	// ADC0->SC3 |= ADC_SC3_ADCO(1U);

	// Software trigger
	// ADC0->SC2 = 0x00;

	// Hardware trigger
	ADC0->SC2 = 0x00;
	ADC0->SC2 |= ADC_SC2_ADTRG(1U);

	// Setup DMA

	// Enable DMA clock
	SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK;
	SIM->SCGC7 |= SIM_SCGC7_DMA_MASK;

	// Disable DMA Mux channel first
	DMAMUX0->CHCFG[0] = 0x00;

	// Configure DMA for ADC operation
	DMA0->DMA[0].SAR = (uint32_t)&ADC0->R[0];				// Source
	DMA0->DMA[0].DAR = (uint32_t)&tls_buffer_A[0];			// Destination

	DMA0->DMA[0].DSR_BCR = DMA_DSR_BCR_BCR(EXPO_NCLK_X2);	// Bytes data to transfer (128 16-bit ADC samples)

	DMA0->DMA[0].DCR |= ( DMA_DCR_EINT_MASK|				// Enable interrupt
				 	 	  DMA_DCR_ERQ_MASK |				// Enable peripheral request
						  DMA_DCR_D_REQ(1U)|				// Disable peripheral request upon completion
						  DMA_DCR_CS_MASK  |				// Single transfer per ADC request
						  DMA_DCR_SSIZE(2U)|				// Set source size to 16 bits
						  DMA_DCR_DINC_MASK|				// Set increments to destination address
						  DMA_DCR_DSIZE(2U));				// Set destination size of 16 bits


	// Enable DMA channel and source
	DMAMUX0->CHCFG[0] |= DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_SOURCE(40);

	// Enable ADC DMA request
	ADC0->SC2 |= ADC_SC2_DMAEN(1U);
}


void BSP_TSL1401_Pins_Init()
{
	// Setup TSL1401 pins
	// SI	-> C4

	SIM->SCGC5 |= SIM_SCGC5_PORTC(1U);		// Start GPIOC clock

	PORTC->PCR[4] &= ~PORT_PCR_MUX_MASK;	// Connect C4 pin to AF1 (GPIO)
	PORTC->PCR[4] |= PORT_PCR_MUX(1U);

	GPIOC->PCOR =  1U <<4U;					// Reset Output C4 (low)
	GPIOC->PDDR |= 1U <<4U;					// Set C4 as output
}
