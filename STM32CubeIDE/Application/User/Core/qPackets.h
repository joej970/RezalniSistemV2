/*
 * qPackets.h
 *
 *  Created on: Feb 21, 2021
 *      Author: Asus
 */

#ifndef APPLICATION_USER_CORE_QPACKETS_H_
#define APPLICATION_USER_CORE_QPACKETS_H_

#include <stdint.h>


typedef struct{
	uint8_t isActive;
	uint8_t immCut;
	uint32_t length;
} qPacket_encoderControl_t;





#endif /* APPLICATION_USER_CORE_QPACKETS_H_ */
