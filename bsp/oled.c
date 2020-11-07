/*
 * oled.c
 *
 *  Created on: 7 mars 2019
 *      Author: Laurent
 */

#include "MKL25Z4.h"
#include "main.h"
#include "font.h"


static void 	OLED_SPI_SendByte		(uint8_t byte);


void	BSP_OLED_SPI_Init()
{
	// OLED SPI Pins
	// SPI1_SCK		-> PE2 (AF2)
	// SPI1_MOSI	-> PE3 (AF5)

	// D/C			-> PE0 (AF1 = GPIO)
	// CS			-> PE4 (AF1 = GPIO)
	// Reset		-> PE5 (AF1 = GPIO)

	// Start GPIOE clock
	SIM->SCGC5 |= SIM_SCGC5_PORTE(1U);

	// Connect PE2 pin to AF2 (SPI1 SCK)
	PORTE->PCR[2] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[2] |= PORT_PCR_MUX(2U);

	// Connect PE3 pin to AF5 (SPI1 MOSI)
	PORTE->PCR[3] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[3] |= PORT_PCR_MUX(5U);


	// Connect PE0, PE4, PE5 to AF1 (GPIO)
	PORTE->PCR[0] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[0] |= PORT_PCR_MUX(1U);

	PORTE->PCR[4] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[4] |= PORT_PCR_MUX(1U);

	PORTE->PCR[5] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[5] |= PORT_PCR_MUX(1U);


	// Default PE0 pin (D/C) state is high
	GPIOE->PSOR =  1U <<0U;

	// Default PE4 pin (CS) state is high
	GPIOE->PSOR =  1U <<4U;

	// Default PE5 pin (Reset) state is high
	GPIOE->PSOR =  1U <<5U;

	// Set PE0, PE4, PE5 pins (D/C, CS, Reset) as output
	GPIOE->PDDR |= 1U <<0U;
	GPIOE->PDDR |= 1U <<4U;
	GPIOE->PDDR |= 1U <<5U;

	// Start SPI1 clock
	SIM->SCGC4 |= SIM_SCGC4_SPI1(1U);

	// Reset SPI1 configuration
	SPI1->C1 = 0x00;
	SPI1->C2 = 0x00;

	// Master mode
	SPI1->C1 |= SPI_C1_MSTR(1U);

	// SS pin is regular GPIO
	SPI1->C1 |= SPI_C1_SSOE(1U);

	// Set Baudrate :	Prescaler to 8 -> 24MHz/6 = 4MHz
	//					Divisor   to 8 -> 4MHz/8  = 500kHz
	SPI1->BR = SPI_BR_SPPR(4U -1U) | SPI_BR_SPR(3U);

	// Enable SPI1 module
	SPI1->C1 |= SPI_C1_SPE(1U);
}



void BSP_OLED_Reset()
{
	GPIOE->PCOR = 1U <<5U;				// _RESET	low
	delay_ms(100);
	GPIOE->PSOR = 1U <<5U;				// _RESET	high
}



void BSP_OLED_Init()
{
	delay_ms(10);

	BSP_OLED_SendCommand(SSD1306_DISPLAYOFF);
	delay_ms(50);

	BSP_OLED_SendCommand(SSD1306_SETDISPLAYCLOCKDIV);
	BSP_OLED_SendCommand(0x80);
	delay_ms(10);

	BSP_OLED_SendCommand(SSD1306_SETMULTIPLEX);
	BSP_OLED_SendCommand(SSD1306_LCDHEIGHT - 1);
	delay_ms(10);

	BSP_OLED_SendCommand(SSD1306_SETDISPLAYOFFSET);
	BSP_OLED_SendCommand(0x00);
	delay_ms(10);

	BSP_OLED_SendCommand(SSD1306_SETSTARTLINE | 0x0);
	delay_ms(10);

	BSP_OLED_SendCommand(SSD1306_CHARGEPUMP);
	BSP_OLED_SendCommand(0x14);						// 0x10 -> External Vcc, 0x14 -> Internal Vcc
	delay_ms(10);

	BSP_OLED_SendCommand(SSD1306_MEMORYMODE);
	BSP_OLED_SendCommand(0x00);
	delay_ms(10);

	//BSP_OLED_SendCommand(SSD1306_SEGREMAP);			// Change here to revert display
	BSP_OLED_SendCommand(0xA1);

	//BSP_OLED_SendCommand(SSD1306_COMSCANINC);		// Change here to revert display
	BSP_OLED_SendCommand(0xC8);

	delay_ms(10);

	BSP_OLED_SendCommand(SSD1306_SETCOMPINS);
	BSP_OLED_SendCommand(0x12);
	delay_ms(10);

	BSP_OLED_SendCommand(SSD1306_SETCONTRAST);
	BSP_OLED_SendCommand(0xCF);						// 0x9F -> External Vcc, 0xCF -> Internal Vcc
	delay_ms(10);

	BSP_OLED_SendCommand(SSD1306_SETPRECHARGE);
	BSP_OLED_SendCommand(0xF1);						// 0x22 -> External Vcc, 0xF1 -> Internal Vcc
	delay_ms(10);

	BSP_OLED_SendCommand(SSD1306_SETVCOMDETECT);
	BSP_OLED_SendCommand(0x40);
	delay_ms(10);

	BSP_OLED_SendCommand(SSD1306_DISPLAYALLON_RESUME);
	//OLED_SendCommand(0xA5);
	delay_ms(10);

	BSP_OLED_SendCommand(SSD1306_NORMALDISPLAY );
	delay_ms(10);

	BSP_OLED_SendCommand(SSD1306_DEACTIVATE_SCROLL);
	delay_ms(10);

	BSP_OLED_SendCommand(0x40);            	//Set Display Start line = 0
	delay_ms(10);

	BSP_OLED_SendCommand(0xB0);          	//Set Display Start Page = 0
	delay_ms(10);

	BSP_OLED_Clear();						// Clear Display
	delay_ms(10);

	BSP_OLED_SendCommand(SSD1306_DISPLAYON);
	delay_ms(10);
}


void BSP_OLED_Clear(void)
{
  uint8_t i,k;

  for(k=0;k<8;k++)
  {
	  BSP_OLED_setXY(k,0);
	  for(i=0;i<128;i++)
	  {
		  BSP_OLED_SendData(0x00);         //clear all COL
	  }
  }
}


void BSP_OLED_setXY(uint8_t row, uint8_t col)
{
	BSP_OLED_SendCommand(0xb0+row);         		//set page address
	BSP_OLED_SendCommand(0x00+(8*col&0x0f));       	//set low col address
	BSP_OLED_SendCommand(0x10+((8*col>>4)&0x0f));  	//set high col address
}


void BSP_OLED_PrintChar(uint8_t byte)
{
	uint8_t	j;

	for (j=0; j<5; j++)
	{
		BSP_OLED_SendData(font5x8[(5*byte)+j]);
	}
	BSP_OLED_SendData(0x00);						// Space between chars
}


void BSP_OLED_SendStr(uint8_t *string)
{
	uint8_t j;

	while (*string)
	{
		for (j=0; j<5; j++)
		{
			BSP_OLED_SendData(font5x8[(*string*5)+j]);
		}
		BSP_OLED_SendData(0x00);						// Space between chars
		string++;
	}
}


void BSP_OLED_SendBMP(uint8_t *bmp)
{
	uint8_t	i,j;

	for(i=0; i<8; i++)			// For each row
	{
		BSP_OLED_SendCommand( 0xB0 + i );				// Set row address

		for(j=0; j<128; j++)	// For each column
		{
			BSP_OLED_SendCommand( 0x00 + (j & 0x0F) );       	//set low col address
			BSP_OLED_SendCommand( 0x10 + ((j>>4) & 0x0F) );  	//set high col address

			BSP_OLED_SendData((uint8_t)*bmp);
			bmp++;
		}
	}
}

void BSP_OLED_SendCommand(uint8_t command)
{
	GPIOE->PCOR = 1U <<0U;			// D/_C 	low
	GPIOE->PCOR = 1U <<4U;			// Drive CS low

	OLED_SPI_SendByte(command);

	GPIOE->PSOR =  1U <<4U;			// Drive CS high
}


void BSP_OLED_SendData(uint8_t data)
{
	GPIOE->PSOR = 1U <<0U;			// D/_C 	high
	GPIOE->PCOR = 1U <<4U;			// Drive CS low

	OLED_SPI_SendByte(data);

	GPIOE->PSOR =  1U <<4U;			// Drive CS high
}



/*
 * 	Sends single byte over SPI
 */

static void OLED_SPI_SendByte(uint8_t byte)
{
	// Wait here until TX buffer is empty
	while((SPI1->S & SPI_S_SPTEF_MASK) != SPI_S_SPTEF_MASK);

	// Send byte
	SPI1->D = byte;

	// Wait here until RX is filled
	while((SPI1->S & SPI_S_SPRF_MASK) != SPI_S_SPRF_MASK);

	// Read byte to clear SPRF flag
	SPI1->D;
}
