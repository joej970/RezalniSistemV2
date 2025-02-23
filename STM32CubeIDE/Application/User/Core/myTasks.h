/*
 * tasks.h
 *
 *  Created on: Feb 20, 2021
 *      Author: Asus
 */

#ifndef APPLICATION_USER_CORE_MYTASKS_H_
#define APPLICATION_USER_CORE_MYTASKS_H_


void encoderControlTask(void *pvParameters);
void reportTask(void *pvParameters);
void relaySetupTask(void *pvParameters);
void singleEventTask(void *pvParameters);
void writeSettingsTask(void *pvParamaters);
void grblCommunicationTask(void *pvParamaters);


#endif /* APPLICATION_USER_CORE_MYTASKS_H_ */
