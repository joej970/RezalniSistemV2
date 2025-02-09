/*
 * qPackages.h
 *
 *  Created on: Feb 21, 2021
 *      Author: Asus
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef APPLICATION_USER_CORE_QPACKAGES_H_
#define APPLICATION_USER_CORE_QPACKAGES_H_


#include <stdint.h>
#include "main.h"



typedef struct{
	uint16_t angle_alpha_01deg;
	uint16_t angle_beta_01deg;
	uint16_t feedrate;
	uint16_t width_01mm;
	uint16_t origin_y0_01mm;
	uint16_t origin_x0_01mm;
	uint8_t slot; //0 for LAST, 1 for Save1, 2 for Save2, 3 for Save3

} qPackage_laserParams_t;


typedef struct{
	char data[UART_RX_GLOBAL_BUFFER_SIZE];
	uint16_t length;
} qPackage_UART_RX;

typedef struct{
	uint8_t isActive;
	uint32_t length_01mm; // unit: 0.1 mm
	uint16_t resolution;
	uint16_t radius_01mm;
} qPackage_encoderControl_t;

typedef struct{
	uint32_t currLength_01mm;
	uint32_t amount;

} qPackage_report_t;

typedef struct{
	uint32_t relayId;
	uint32_t delay_ms;
	uint32_t duration_ms;
} qPackage_relaySetup_t;

typedef struct{
	enum statusId_t statusId;
	uint32_t data;
	char message[30];
} qPackage_statusReport_t;

typedef struct{
	qPackage_encoderControl_t encoderControl;
	qPackage_relaySetup_t relay1;
	qPackage_relaySetup_t relay2;
	qPackage_relaySetup_t relay3;
	qPackage_laserParams_t laserParams;
	uint8_t languageIdx;
	uint8_t relaysActive;
	uint16_t brightness;
	uint16_t settingsMask;

} qPackage_settings_t;





#endif /* APPLICATION_USER_CORE_QPACKAGES_H_ */

#ifdef __cplusplus
}
#endif
