/*
 * accl.h
 *
 *  Created on: 9 mars 2019
 *      Author: Laurent
 */

#ifndef ACCL_H_
#define ACCL_H_

#include "main.h"

void	vTaskACCL (void *pvParameters);

extern 	xTaskHandle			vTaskACCL_handle;
extern 	xSemaphoreHandle	xACCLSem;

#endif /* ACCL_H_ */
