/*
 * mma.c
 *
 *  Created on: 3 mars 2019
 *      Author: Laurent
 */

#include "MKL25Z4.h"
#include "main.h"

uint8_t	MMA8451_I2C_Init()
{
	// I2C0 SCL -> PE24
	// I2C0 SDA -> PE25

	// Start GPIOE clock
	SIM->SCGC5 |= SIM_SCGC5_PORTE(1U);

	// Connect PE24 pin to AF5 (I2C0 SCL)
	PORTE->PCR[24] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[24] |= PORT_PCR_MUX(5U);

	// Connect PE25 pin to AF5 (I2C0 SDA)
	PORTE->PCR[25] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[25] |= PORT_PCR_MUX(5U);

	// Start I2C0 clock
	SIM->SCGC4 |= SIM_SCGC4_I2C0(1U);

	// Initialize control registers
	I2C0->C1 = 0x00;
	I2C0->C2 = 0x00;

	// Set Baudrate to 24MHz/80 = 300kHz (ICR = 0x14, MULT = 0)
	I2C0->F = 0x14;

	// Enable I2C0 operation
	I2C0->C1 |= I2C_C1_IICEN(1U);

	return 0;
}


void MMA8451_INT_Pin_Init()
{
	// MMA8451 INT -> PA14

	// Start GPIOA clock
	SIM->SCGC5 |= SIM_SCGC5_PORTA(1U);

	// Set A14 as input
	GPIOA->PDDR &= ~(1U <<14U);

	// Connect A14 pin to AF1 (GPIO)
	PORTA->PCR[14] &= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[14] |= PORT_PCR_MUX(1U);

	// Interrupt on falling edge
	PORTA->PCR[14] &= ~PORT_PCR_IRQC_MASK;
	PORTA->PCR[14] |= PORT_PCR_IRQC(10U);

	// Clear Interrupt flag
	PORTA->PCR[14] |= PORT_PCR_ISF(1U);
}



uint8_t	MMA8451_ReadByte(uint8_t add)
{

	uint8_t		byte;

	// Generate START with TX mode
	I2C0->C1 |= I2C_C1_TX(1U) | I2C_C1_MST(1U);

	// Transmit slave address with WRITE bit
	I2C0->D = (MMA845x_I2C_ADDRESS <<1U) | 0x00;

	// Wait for Acknowledge
	while ((I2C0->S & I2C_S_IICIF_MASK) == 0);
	I2C0->S |= I2C_S_IICIF(1U);

	// Transmit slave register to read
	I2C0->D = add;

	// Wait for Acknowledge
	while ((I2C0->S & I2C_S_IICIF_MASK) == 0);
	I2C0->S |= I2C_S_IICIF(1U);

	// Repeat START
	I2C0->C1 |= I2C_C1_RSTA_MASK;

	// Transmit slave address with READ bit
	I2C0->D = (MMA845x_I2C_ADDRESS <<1U) | 0x01;

	// Wait for Acknowledge
	while ((I2C0->S & I2C_S_IICIF_MASK) == 0);
	I2C0->S |= I2C_S_IICIF(1U);

	// Enter RX mode
	I2C0->C1 &= ~I2C_C1_TX_MASK;

	// Disable Acknowledge
	I2C0->C1 |= I2C_C1_TXAK_MASK;

	// Start transfer
	byte = I2C0->D;

	// Wait
	while ((I2C0->S & I2C_S_IICIF_MASK) == 0);
	I2C0->S |= I2C_S_IICIF(1U);

	// Clear Master operation (generates a STOP condition)
	I2C0->C1 &= ~I2C_C1_MST_MASK;
	I2C0->C1 &= ~I2C_C1_TX_MASK;

	// Read response
	byte = I2C0->D;

	return byte;
}


uint8_t MMA8451_ReadNBytes(uint8_t add, uint8_t nbytes, uint8_t* buffer)
{
	uint8_t	i;

	// Generate START with TX mode
	I2C0->C1 |= I2C_C1_TX(1U) | I2C_C1_MST(1U);

	// Transmit slave address with WRITE bit
	I2C0->D = (MMA845x_I2C_ADDRESS <<1U) | 0x00;

	// Wait for Acknowledge
	while ((I2C0->S & I2C_S_IICIF_MASK) == 0);
	I2C0->S |= I2C_S_IICIF(1U);

	// Transmit slave register to read
	I2C0->D = add;

	// Wait for Acknowledge
	while ((I2C0->S & I2C_S_IICIF_MASK) == 0);
	I2C0->S |= I2C_S_IICIF(1U);

	// Repeat START
	I2C0->C1 |= I2C_C1_RSTA_MASK;

	// Transmit slave address with READ bit
	I2C0->D = (MMA845x_I2C_ADDRESS <<1U) | 0x01;

	// Wait for Acknowledge
	while ((I2C0->S & I2C_S_IICIF_MASK) == 0);
	I2C0->S |= I2C_S_IICIF(1U);

	// Enter RX mode
	I2C0->C1 &= ~I2C_C1_TX_MASK;

	// Enable Acknowledge
	I2C0->C1 &= ~I2C_C1_TXAK_MASK;

	// Start transfer
	i = I2C0->D;

	// Wait
	while ((I2C0->S & I2C_S_IICIF_MASK) == 0);
	I2C0->S |= I2C_S_IICIF(1U);

	for (i=0; i<(nbytes-2); i++)
	{
		// Read bytes
		*buffer = I2C0->D;
		buffer++;

		// Wait
		while ((I2C0->S & I2C_S_IICIF_MASK) == 0);
		I2C0->S |= I2C_S_IICIF(1U);
	}

	// Disable Acknowledge
	I2C0->C1 |= I2C_C1_TXAK_MASK;

	// Read byte
	*buffer = I2C0->D;
	buffer++;

	// Wait
	while ((I2C0->S & I2C_S_IICIF_MASK) == 0);
	I2C0->S |= I2C_S_IICIF(1U);

	// Clear Master operation (generates a STOP condition)
	I2C0->C1 &= ~I2C_C1_MST_MASK;
	I2C0->C1 &= ~I2C_C1_TX_MASK;

	// Read last byte
	*buffer = I2C0->D;

	return 0;
}



uint8_t	MMA8451_WriteByte(uint8_t add, uint8_t byte)
{
	// Generate START with TX mode
	I2C0->C1 |= I2C_C1_TX(1U) | I2C_C1_MST(1U);

	// Transmit slave address with WRITE bit
	I2C0->D = (MMA845x_I2C_ADDRESS <<1U) | 0x00;

	// Wait for Acknowledge
	while ((I2C0->S & I2C_S_IICIF_MASK) == 0);
	I2C0->S |= I2C_S_IICIF(1U);

	// Transmit register address
	I2C0->D = add;

	// Wait for Acknowledge
	while ((I2C0->S & I2C_S_IICIF_MASK) == 0);
	I2C0->S |= I2C_S_IICIF(1U);

	// Transmit data
	I2C0->D = byte;

	// Wait for Acknowledge
	while ((I2C0->S & I2C_S_IICIF_MASK) == 0);
	I2C0->S |= I2C_S_IICIF(1U);

	// Clear Master operation (generates a STOP condition)
	I2C0->C1 &= ~I2C_C1_MST_MASK;
	I2C0->C1 &= ~I2C_C1_TX_MASK;

	return 0;
}



void MMA8451_Init()
{
	uint8_t data = 0;

	MMA8451_WriteByte(CTRL_REG2, 0x40);				// Reset all registers to POR values
	delay_ms(10);

	do		// Wait for the RST bit to clear
	{
		data = MMA8451_ReadByte(CTRL_REG2) & 0x40;
		delay_ms(10);
	} 	while (data);


	//my_printf("Configuring MMA8451\r\n");


	MMA8451_WriteByte(CTRL_REG1, 0x00);				// Standby mode to allow configuration
	delay_ms(2);
	//my_printf("CTRL_REG1 = 0x%02x\r\n", MMA8451_ReadByte(CTRL_REG1));

	MMA8451_WriteByte(XYZ_DATA_CFG_REG, 0x00);		// +/-2g range -> 1g = 16384/4 = 4096 counts
	delay_ms(2);
	//my_printf("XYZ_DATA_CFG_REG = 0x%02x\r\n", MMA8451_ReadByte(XYZ_DATA_CFG_REG));

	MMA8451_WriteByte(CTRL_REG2, 0x02);				// High Resolution mode
	delay_ms(2);
	//my_printf("CTRL_REG2 = 0x%02x\r\n", MMA8451_ReadByte(CTRL_REG2));

	MMA8451_WriteByte(CTRL_REG3, 0x00);				// Push-pull, active low interrupt
	delay_ms(2);
	//my_printf("CTRL_REG3 = 0x%02x\r\n", MMA8451_ReadByte(CTRL_REG3));

	MMA8451_WriteByte(CTRL_REG4, 0x01);				// Enable DRDY interrupt
	delay_ms(2);
	//my_printf("CTRL_REG4 = 0x%02x\r\n", MMA8451_ReadByte(CTRL_REG4));

	MMA8451_WriteByte(CTRL_REG5, 0x01);				// DRDY interrupt routed to INT1 - PTA14
	delay_ms(2);
	//my_printf("CTRL_REG5 = 0x%02x\r\n", MMA8451_ReadByte(CTRL_REG5));

	MMA8451_WriteByte(CTRL_REG1, 0x19);				// ODR = 100Hz, Active mode
	delay_ms(2);
	//my_printf("CTRL_REG1 = 0x%02x\r\n", MMA8451_ReadByte(CTRL_REG1));
}




