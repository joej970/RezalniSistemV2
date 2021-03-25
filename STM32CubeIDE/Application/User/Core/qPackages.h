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


//#include "status_enum.h"
#include "main.h"





//#include "../../STM32CubeIDE/Application/User/Core/status_enum.h"
typedef struct{
	uint8_t isActive;
	//uint8_t immCut;
	uint32_t length_01mm; // unit: 0.1 mm
	uint16_t resolution;
	uint16_t radius_01mm;
} qPackage_encoderControl_t;

typedef struct{
	//uint32_t setLengthActual_01mm;
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
	//char message[30];
} qPackage_statusReport_t;

typedef struct{
	qPackage_encoderControl_t encoderControl;
	qPackage_relaySetup_t relay1;
	qPackage_relaySetup_t relay2;
	qPackage_relaySetup_t relay3;
	uint8_t settingsMask;
} qPackage_settings_t;





#endif /* APPLICATION_USER_CORE_QPACKAGES_H_ */

#ifdef __cplusplus
}
#endif
