/*
 * bsp.h
 *
 *  Created on: 23 févr. 2019
 *      Author: Laurent
 */

#ifndef BSP_H_
#define BSP_H_

#define RED		0
#define YELLOW	1
#define GREEN	2
#define CYAN	3
#define BLUE	4
#define MAGENTA	5
#define WHITE	6


void	BSP_LED_Pin_Init		(void);
void	BSP_LED_On				(uint8_t color);
void	BSP_LED_Off				(void);

void 	BSP_DBG_Pin_Init		(void);

void	BSP_Console_Init		(void);
void 	BSP_BT_Console_Init		(void);

void 	BSP_LPTMR0_Init			(void);		// Speed encoder Right
void 	BSP_TPM2_Init			(void);		// Speed encoder Left

void	BSP_TPM1_PWM_Init		(void);		// Steering Channel
void 	BSP_TPM0_PWM_Init		(void);		// 4-Channels Motor control

void 	BSP_ADC_DMA_Init		(void);
void 	BSP_TSL1401_Pins_Init	(void);





#endif /* BSP_H_ */
