/*
 * tasks.h
 *
 *  Created on: Feb 20, 2021
 *      Author: Asus
 */

#ifndef APPLICATION_USER_CORE_MYTASKS_H_
#define APPLICATION_USER_CORE_MYTASKS_H_

//#include "app_touchgfx.h"
#include <qPackages.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "stm32f7xx_hal.h"

extern QueueHandle_t qhGUItoEncoderControl;
extern QueueHandle_t qhEncoderControlToReport;
extern QueueHandle_t qhReportToTouchGFX;

void encoderControlTask(void *pvParameters);
void reportTask(void *pvParameters);
//void StartTouchGFXTask(void * argument);

#endif /* APPLICATION_USER_CORE_MYTASKS_H_ */
