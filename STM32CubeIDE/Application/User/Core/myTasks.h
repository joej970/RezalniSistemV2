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
double get_filtered_velocity(double new_value);

void configureForHardwareTimer(GPIO_TypeDef* port, uint16_t pin, uint8_t alternateFunction);
void configureForSoftwareTimer(GPIO_TypeDef* port, uint16_t pin);
void disableRelayTimers(void);
void enableRelayTimers(void);
void suspendRelayHardwareTimer(int8_t idx);
void resumeRelayHardwareTimer(int8_t idx);


#endif /* APPLICATION_USER_CORE_MYTASKS_H_ */
