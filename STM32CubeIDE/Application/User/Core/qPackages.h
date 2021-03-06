/*
 * qPackages.h
 *
 *  Created on: Feb 21, 2021
 *      Author: Asus
 */

#ifndef APPLICATION_USER_CORE_QPACKAGES_H_
#define APPLICATION_USER_CORE_QPACKAGES_H_

#include <stdint.h>


typedef struct{
	uint8_t isActive;
	//uint8_t immCut;
	uint32_t length_01mm; // unit: 0.1 mm
	uint32_t resolution;
	uint32_t radius_01mm;
} qPackage_encoderControl_t;


// might add cuttingActive
typedef struct{
	//uint32_t setLengthActual_01mm;
	uint32_t currLength_01mm;
	uint32_t ammount;

} qPackage_report_t;

typedef struct{
	uint32_t relayId;
	uint32_t delay_ms;
	uint32_t duration_ms;
} qPackage_relaySetup_t;

typedef struct{
	uint32_t statusId;
	uint32_t data;
	//char message[30];
} qPackage_statusReport_t;




#endif /* APPLICATION_USER_CORE_QPACKAGES_H_ */
