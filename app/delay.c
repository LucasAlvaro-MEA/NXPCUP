
#include "delay.h"

// Basic delay function


void delay_ms(uint32_t delay)
{
	uint32_t	i;
	for(i=0; i<(delay*10000); i++);		// Tuned for ms on STM32L432 at 80MHz
}

void delay_us(uint32_t delay)
{
	uint32_t	i;
	for(i=0; i<(delay*3); i++);			// Tuned for ms on STM32L432 at 80MHz
}
