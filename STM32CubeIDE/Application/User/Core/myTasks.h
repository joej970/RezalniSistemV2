/*
 * tasks.h
 *
 *  Created on: Feb 20, 2021
 *      Author: Asus
 */

#ifndef APPLICATION_USER_CORE_MYTASKS_H_
#define APPLICATION_USER_CORE_MYTASKS_H_

#include "app_touchgfx.h"
#include "qPackets.h"
#include "FreeRTOS.h"
#include "queue.h"

extern QueueHandle_t qhGUItoEncoderControl;

void encoderControlTask(void *pvParameters);
void StartTouchGFXTask(void * argument);

#endif /* APPLICATION_USER_CORE_MYTASKS_H_ */
