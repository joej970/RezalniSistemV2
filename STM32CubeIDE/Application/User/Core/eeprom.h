/*
 * eeprom.h
 *
 *  Created on: Apr 3, 2021
 *      Author: Asus
 */

#ifndef APPLICATION_USER_CORE_EEPROM_H_
#define APPLICATION_USER_CORE_EEPROM_H_

#include <stdint.h>
#include "stm32f7xx_hal.h"

enum eepromStatus_t{
	EEPROM_SUCCESS,
	EEPROM_TIMEOUT_1,
	EEPROM_TIMEOUT_2,
	EEPROM_TIMEOUT_3,
	EEPROM_TIMEOUT_4,
	EEPROM_BUSY,
	EEPROM_TXFULL,
	EEPROM_ERR
};




enum eepromStatus_t byteWriteToEEPROM(uint8_t dataAddr, uint8_t data);
enum eepromStatus_t bytesWriteToEEPROM(uint8_t dataAddr, uint8_t *srcBuffer, uint8_t nr);
enum eepromStatus_t bytesReadFromEEPROM(uint8_t dataAddr, uint8_t *dstBuffer, uint8_t nr);
enum eepromStatus_t getResolutionRadiusFromEEPROM(uint16_t*, uint16_t*);
enum eepromStatus_t getSettingsFromEEPROM(
				uint16_t* resolution, uint16_t* radius_01mm, uint32_t* setLength_01mm, uint32_t *relayData[6],
				uint8_t* languageIdx, uint8_t* relaysActive, uint16_t* brightness);
enum eepromStatus_t saveRelayDataToEEPROM(uint32_t* duration, uint32_t* delay, uint8_t relayIdx);
enum eepromStatus_t saveSetLengthToEEPROM(uint32_t* setLength);
enum eepromStatus_t saveResolutionRadiusToEEPROM(uint16_t* resolution, uint16_t* radius_01mm);
enum eepromStatus_t saveLanguageIdxToEEPROM(uint8_t languageIdx);
enum eepromStatus_t saveRelaysActiveToEEPROM(uint8_t relaysActive);

#endif /* APPLICATION_USER_CORE_EEPROM_H_ */
