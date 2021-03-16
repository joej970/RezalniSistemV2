/*
 * tasks.h
 *
 *  Created on: Feb 20, 2021
 *      Author: Asus
 */

#ifndef APPLICATION_USER_CORE_MYTASKS_H_
#define APPLICATION_USER_CORE_MYTASKS_H_



//#include "app_touchgfx.h"

//#include <qPackages.h>

#include "queue.h"

//#include "status_enum.h"




extern QueueHandle_t qhGUItoEncoderControl;
extern QueueHandle_t qhEncoderControlToReport;
extern QueueHandle_t qhReportToTouchGFX;
extern QueueHandle_t qhTouchGFXToRelaySetup;
extern QueueHandle_t qhStatusReport;

void encoderControlTask(void *pvParameters);
void reportTask(void *pvParameters);
void relaySetupTask(void *pvParameters);
void singleEventTask(void *pvParameters);
//void StartTouchGFXTask(void * argument);



#endif /* APPLICATION_USER_CORE_MYTASKS_H_ */
