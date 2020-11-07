/*
 * cam.h
 *
 *  Created on: 8 mars 2019
 *      Author: Laurent
 */

#ifndef CAM_H_
#define CAM_H_

#include "main.h"
#include "bsp.h"

void	vTaskCAM (void *pvParameters);

extern 	xTaskHandle			vTaskCAM_handle;
extern 	xSemaphoreHandle	xCAMSem;


#endif /* CAM_H_ */
