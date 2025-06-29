/*
 * eeprom.c
 *
 * Chip: 24FC01T-E/OT
 *
 *  Created on: Apr 3, 2021
 *      Author: Asus
 */

#include "eeprom.h"
#include <stdlib.h>
#include <string.h>

//#include "main.h"

#define EEPROM_ADDR		(uint8_t)0b10100000




const char* eepromStatus_strings[]  = {
				"EEPROM_SUCCESS",
				"EEPROM_TIMEOUT_2",
				"EEPROM_TIMEOUT_3",
				"EEPROM_TIMEOUT_3",
				"EEPROM_TIMEOUT_4",
				"EEPROM_BUSY",
				"EEPROM_TXFULL",
				"EEPROM_ERR",
				"EEPROM_MAX_8_BYTES",
				"EEPROM_ERR_NOT_ALIGNED"
};

enum eepromMemoryMap_t{
//	RES_ENTRYIDX_RADIUS_ENTRYVALID = 0,
	RES_RAD_4		= 0,
	SET_LEN_4 		= 4,
	RLY1_DUR_4 		= 8,
	RLY1_DEL_4		= 12,
	RLY2_DUR_4		= 16,
	RLY2_DEL_4 		= 20,
	RLY3_DUR_4 		= 24,
	RLY3_DEL_4 		= 28,
	LANG_IDX_1		= 32,
	RLY_ACTIVE_1	= 33,
	BRIGHTNESS_2	= 34,
	CIRCUMFERENCE_2	= 36,
	LASER_PARAMS_LAST_8 = 40,
	LASER_PARAMS_SLOT_1_8 = 48,
	LASER_PARAMS_SLOT_2_8 = 56,
	LASER_PARAMS_SLOT_3_8 = 68,
	LASER_PARAMS_ALPHA_CUT_EN_LAST_1 = 76,
	LASER_PARAMS_ALPHA_CUT_EN_SLOT_1_1 = 77,
	LASER_PARAMS_ALPHA_CUT_EN_SLOT_2_1 = 78,
	LASER_PARAMS_ALPHA_CUT_EN_SLOT_3_1 = 79,
	LASER_ORIGIN_LAST_4 = 80,
	LASER_ORIGIN_SLOT_1_4 = 84,
	LASER_ORIGIN_SLOT_2_4 = 88,
	LASER_ORIGIN_SLOT_3_4 = 92,
	LASER_CONSOLE_LAST_1 = 96,
	MEM_SIZE		= 97
};

//enum eepromMemoryMap_t{
////	RES_ENTRYIDX_RADIUS_ENTRYVALID = 0,
//	RES_16_8,
//	RES_7_0,
//	RADIUS_16_8,
//	RADIUS_7_0,
//	SET_LENGTH_31_24,
//	SET_LENGTH_23_16,
//	SET_LENGTH_15_8,
//	SET_LENGTH_7_0,
//
//	RLY1_DUR_31_24,
//	RLY1_DUR_23_16,
//	RLY1_DUR_15_8,
//	RLY1_DUR_7_0,
//	RLY1_DEL_31_24,
//	RLY1_DEL_23_16,
//	RLY1_DEL_15_8,
//	RLY1_DEL_7_0,
//
//	RLY2_DUR_31_24,
//	RLY2_DUR_23_16,
//	RLY2_DUR_15_8,
//	RLY2_DUR_7_0,
//	RLY2_DEL_31_24,
//	RLY2_DEL_23_16,
//	RLY2_DEL_15_8,
//	RLY2_DEL_7_0,
//
//	RLY3_DUR_31_24,
//	RLY3_DUR_23_16,
//	RLY3_DUR_15_8,
//	RLY3_DUR_7_0,
//	RLY3_DEL_31_24,
//	RLY3_DEL_23_16,
//	RLY3_DEL_15_8,
//	RLY3_DEL_7_0,
//
//	LANG_IDX_7_0,
//	RLY_ACTIVE_7_0,
//	BRIGHTNESS_15_8,
//	BRIGHTNESS_7_0,
//
//	MEM_SIZE	= 36
//};



/*
 * Send <nr> of bytes to EEPROM over I2C1. Blocking function
 * */
enum eepromStatus_t bytesWriteToEEPROM(uint8_t dataAddr, uint8_t *srcBuffer, uint8_t nr){


	if(dataAddr % nr != 0){
		// address is not nr-byte aligned: internal address counter increments last 3 bits
		return EEPROM_ERR_NOT_ALIGNED;
	}

	if(nr > 8){
		return EEPROM_MAX_8_BYTES;
	}


	// TODO: should first read from location and check if write is even required
	uint8_t* checkBuffer = malloc(nr * sizeof(uint8_t));
	enum eepromStatus_t status = bytesReadFromEEPROM(dataAddr, checkBuffer, nr);
	if(status == EEPROM_SUCCESS){
		if(0 == memcmp(srcBuffer, checkBuffer, (size_t)nr)){ // if data is equal, do not perform write
			free(checkBuffer);
			return EEPROM_SUCCESS;
		}
	}else{
		// if read failed, something is wrong
		free(checkBuffer);
		return status;
	}
	free(checkBuffer); // data is different. continue with writing.




	static uint32_t lastWriteTimestamp = 0;
	//	Wait for 5ms after last write
	while(lastWriteTimestamp + 5 >= HAL_GetTick()){
	}
	uint32_t startTime = HAL_GetTick();
	//  Wait if busy
	while(I2C1->ISR & I2C_ISR_BUSY_Msk) {
		if(startTime + 100 < HAL_GetTick()) {
			I2C1->CR1 &= ~I2C_CR1_PE;
			for(uint8_t i = 0; i<10; i++){
				asm("NOP");
			}
			I2C1->CR1 |= I2C_CR1_PE;
			return EEPROM_BUSY;
		}
	}
	uint32_t icr = I2C1->ICR;
	(void) icr;
	uint32_t tries = 0;
	I2C1->ICR = I2C_ICR_NACKCF_Msk;
	// 	AUTOEND: STOP is sent after NBYTES are transferred; NBYTES: data address + data; SADD: slave address; WRITE = 0
	//I2C1->CR2 = I2C_CR2_AUTOEND | (uint8_t) (nr+1) << I2C_CR2_NBYTES_Pos | EEPROM_ADDR << I2C_CR2_SADD_Pos;

	while(1){
		tries++;
		//	Send START, AUTOEND: STOP is sent after NBYTES are transferred; NBYTES: data address + data; SADD: slave address; WRITE = 0
		I2C1->CR2 = I2C_CR2_START | I2C_CR2_AUTOEND | (uint8_t) (nr+1) << I2C_CR2_NBYTES_Pos | EEPROM_ADDR << I2C_CR2_SADD_Pos;
		//	Wait until address + write bit transmission is finished to check if EEPROM has acknoledged
		while(I2C1->CR2 & I2C_CR2_START_Msk){
			if(startTime + 100 < HAL_GetTick()) {
				return EEPROM_TIMEOUT_1;
			}
		}
		//	Wait for TXIS (ack received) or NACK (nack received)
		while(1){
			if((I2C1->ISR & I2C_ISR_TXIS_Msk) || (I2C1->ISR & I2C_ISR_NACKF_Msk)){
				break;
			}
		}

		//	If NACK is not set (ACK was received) break out of loop
		if(!(I2C1->ISR & I2C_ISR_NACKF_Msk)){
			break;
		}
		// 	Clear NACK & STOP flag for the next attempt
		I2C1->ICR = I2C_ICR_NACKCF_Msk | I2C_ICR_STOPCF_Msk;
		HAL_Delay(1);
	}
	// 	Transfer data address
	I2C1->TXDR = (uint32_t)dataAddr;

	/* Transfer data */
	for(uint8_t i = 0; i < nr; i++){
		//	Wait if next byte needs to be written to TXDR
			while(!(I2C1->ISR & I2C_ISR_TXIS_Msk)){
				if(startTime + 100 < HAL_GetTick()) {
					return EEPROM_TIMEOUT_2;
				}
			}
			// 	Transfer data
			I2C1->TXDR = (uint32_t)srcBuffer[i];
	}

	uint32_t blabla = 0;
	// 	Wait if STOPF is not set
	while(!(I2C1->ISR & I2C_ISR_STOPF_Msk)){
		blabla++;
		if(startTime + 100 < HAL_GetTick()) {
			return EEPROM_TIMEOUT_3;
		}
	}
	//	Clear STOP flag
	I2C1->ICR = I2C_ICR_STOPCF_Msk;
	I2C1->CR2 = 0;

	//HAL_Delay(10);
	//	Timestamp last write
	lastWriteTimestamp = HAL_GetTick();
	return EEPROM_SUCCESS;

}

/*
 * Set EEPROM to <dataAddr> and read <nr> of bytes from EEPROM and load it to <*dstBuffer>. Blocking function (max 100ms).
 * */
enum eepromStatus_t bytesReadFromEEPROM(uint8_t dataAddr, uint8_t *dstBuffer, uint8_t nr){
	uint32_t startTime = HAL_GetTick();
	//  Wait if busy
	while(I2C1->ISR & I2C_ISR_BUSY_Msk) {
		if(HAL_GetTick() - startTime > 100) {
			I2C1->CR1 &= ~I2C_CR1_PE;
			for(uint8_t i = 0; i<10; i++){
				asm("NOP");
			}
			I2C1->CR1 |= I2C_CR1_PE;
			return EEPROM_BUSY;
		}
	}

	/* Has EEPROM finished with previous write cycle?*/
	/* Set data address */

	//I2C1->CR2 = (uint8_t) 1 << I2C_CR2_NBYTES_Pos | EEPROM_ADDR << I2C_CR2_SADD_Pos;
	uint32_t nrOfAttemtps = 0;
	while(1){
		nrOfAttemtps++;
		//	Send START; NBYTES: data address; SADD: slave address; WRITE = 0
		I2C1->CR2 = I2C_CR2_START | (uint8_t) 1 << I2C_CR2_NBYTES_Pos | EEPROM_ADDR << I2C_CR2_SADD_Pos;
		//	Wait until address + write bit is sent to check if EEPROM has acknowledged
		while(I2C1->CR2 & I2C_CR2_START_Msk){
			if(HAL_GetTick() - startTime > 100) {
				return EEPROM_TIMEOUT_1;
			}
		}
		//	If NACK is not set (ACK was received) break out of loop
		if(!(I2C1->ISR & I2C_ISR_NACKF_Msk)){
			break;
		}
		// 	Clear NACK & STOP flag for the next attempt
		I2C1->ICR = I2C_ICR_NACKCF_Msk | I2C_ICR_STOPCF_Msk;
		HAL_Delay(1);
	}
//	//SET
//	TRIGGER_OUT_GPIO_Port->BSRR |= TRIGGER_OUT_Pin;
//	//RESET
//	TRIGGER_OUT_GPIO_Port->BSRR |= (TRIGGER_OUT_Pin) << 16;

	//	Send address
	I2C1->TXDR = (uint32_t)dataAddr;

	// 	Wait if TC bit is not set
	while(!(I2C1->ISR & I2C_ISR_TC_Msk)){
		if(HAL_GetTick() - startTime > 100) {
			return EEPROM_TIMEOUT_2;
		}
	}
	/* Read bytes */
	//  Send START;	AUTOEND: STOP is sent after NBYTES are transferred; NBYTES: data; RD_WRN: read opearation; SADD: slave address;
//	I2C1->CR2 = I2C_CR2_AUTOEND | nr << I2C_CR2_NBYTES_Pos | I2C_CR2_RD_WRN | EEPROM_ADDR << I2C_CR2_SADD_Pos;
	I2C1->ICR = I2C_ICR_STOPCF_Msk;
	I2C1->CR2 = I2C_CR2_START | I2C_CR2_AUTOEND | nr << I2C_CR2_NBYTES_Pos | I2C_CR2_RD_WRN | EEPROM_ADDR << I2C_CR2_SADD_Pos;
	/* Have NBYTES received */
	for(uint8_t i = 0; i < nr; i++){
		// Wait if receive buffer is not not-empty = wait if receive buffer not full
		while(!(I2C1->ISR & I2C_ISR_RXNE)){
			if(HAL_GetTick() - startTime > 100){
				return EEPROM_TIMEOUT_3;
			}
		}
		dstBuffer[i] = I2C1->RXDR;
	}

//	// 	Wait if TC bit is not set
//	while(!(I2C1->ISR & I2C_ISR_TC_Msk)){
//		if(startTime + 100 < HAL_GetTick()) {
//			return EEPROM_TIMEOUT;
//		}
//	}
//	if(I2C1->ISR & I2C_ISR_STOPF_Msk){
//		// should not be set yet
//		uint32_t isr_early = I2C1->ISR;
//		UNUSED(isr_early);
//		uint32_t smt;
//		UNUSED(smt);
//		return EEPROM_ERR;
//	}
//
//	// Stop the transfer now
//	I2C1->CR2 = I2C_CR2_STOP;

	uint32_t blabla = 0;
	// 	Wait if STOPF is not set
	while(1){
		if(I2C1->ISR & I2C_ISR_STOPF_Msk){
			break;
		}
		if(HAL_GetTick() - startTime > 100) {
			return EEPROM_TIMEOUT_4;
		}
		blabla++;
	}
	//	Clear STOP
	I2C1->ICR = I2C_ICR_STOPCF_Msk;
	I2C1->CR2 = 0;

//	if(I2C1->ISR & I2C_ISR_BUSY_Msk) {
//			uint32_t isr = I2C1->ISR;
//			UNUSED(isr);
//			return EEPROM_BUSY;
//		}
	return EEPROM_SUCCESS;
}

enum eepromStatus_t getResolutionRadiusFromEEPROM(uint16_t* resolution, uint16_t* radius_01mm){
	enum eepromStatus_t status = EEPROM_SUCCESS;
	uint8_t dstBuffer[] = {0,0,0,0};
	uint8_t nr = sizeof(dstBuffer)/sizeof(dstBuffer[0]);

	status = bytesReadFromEEPROM((uint8_t)RES_RAD_4, dstBuffer, nr);
	if(status == EEPROM_SUCCESS){
		//	Save resolution, radius data and entryIdx
		*resolution = (uint16_t) (dstBuffer[0] << 8 | dstBuffer[1]);
		*radius_01mm= (uint16_t) (dstBuffer[2] << 8 | dstBuffer[3]);
	}
	return status;
}

enum eepromStatus_t getLaserConsoleActiveFromEEPROM(uint8_t* laserConsoleActive){
	enum eepromStatus_t status = EEPROM_SUCCESS;
	uint8_t dstBuffer[] = {0};
	uint8_t nr = sizeof(dstBuffer)/sizeof(dstBuffer[0]);

	status = bytesReadFromEEPROM((uint8_t)LASER_CONSOLE_LAST_1, dstBuffer, nr);
	if(status == EEPROM_SUCCESS){
		//	Save resolution, radius data and entryIdx
		*laserConsoleActive = (uint8_t) (dstBuffer[0]);
	}
	return status;
}


enum eepromStatus_t getSettingsFromEEPROM(
				uint16_t* resolution, uint16_t* radius_01mm, uint16_t* circumference_01mm, uint32_t* setLength_01mm, uint32_t *relayData[6],
				uint8_t* languageIdx, uint8_t* relaysActive, uint16_t* brightness, qPackage_laserParams_t* laserParams_last){
	enum eepromStatus_t status = EEPROM_SUCCESS;
	uint8_t dstBuffer[MEM_SIZE];
	uint8_t nr = sizeof(dstBuffer)/sizeof(dstBuffer[0]);

	status = bytesReadFromEEPROM((uint8_t)RES_RAD_4, dstBuffer, nr);
	if(status == EEPROM_SUCCESS){
		//	Save resolution, radius data and setLength
		*resolution 	= (uint16_t) (dstBuffer[RES_RAD_4+0] << 8 | dstBuffer[RES_RAD_4+1]);
		*radius_01mm	= (uint16_t) (dstBuffer[RES_RAD_4+2] << 8 | dstBuffer[RES_RAD_4+3]);
		*setLength_01mm = (uint32_t) (dstBuffer[SET_LEN_4+0] << 8 | dstBuffer[SET_LEN_4+1] << 16 | dstBuffer[SET_LEN_4+2] << 8 | dstBuffer[SET_LEN_4+3]);
		*circumference_01mm	= (uint16_t) (dstBuffer[CIRCUMFERENCE_2+0] << 8 | dstBuffer[CIRCUMFERENCE_2+1]);
		//	Save relay data to an array of pointers
		for(int8_t i = 0; i < 6; i++){
			*relayData[i]	= (uint32_t) (dstBuffer[RLY1_DUR_4+0+i*4] << 8 | dstBuffer[RLY1_DUR_4+1+i*4] << 16 | dstBuffer[RLY1_DUR_4+2+i*4] << 8 | dstBuffer[RLY1_DUR_4+3+i*4]);
//			*relayData[i]	= (uint32_t) (dstBuffer[8+i*4] << 8 | dstBuffer[9+i*4] << 16 | dstBuffer[10+i*4] << 8 | dstBuffer[11+i*4]);
		}
		*languageIdx	= (uint8_t)(dstBuffer[LANG_IDX_1]);
		*relaysActive	= (uint8_t)(dstBuffer[RLY_ACTIVE_1]);
		*brightness		= (uint16_t)(dstBuffer[BRIGHTNESS_2+0] << 8 | dstBuffer[BRIGHTNESS_2+1]);
		laserParams_last->angle_alpha_01deg = (uint16_t)(dstBuffer[LASER_PARAMS_LAST_8+0] << 8 | dstBuffer[LASER_PARAMS_LAST_8+1]);
		laserParams_last->angle_beta_01deg  = (uint16_t)(dstBuffer[LASER_PARAMS_LAST_8+2] << 8 | dstBuffer[LASER_PARAMS_LAST_8+3]);
		laserParams_last->feedrate          = (uint16_t)(dstBuffer[LASER_PARAMS_LAST_8+4] << 8 | dstBuffer[LASER_PARAMS_LAST_8+5]);
		laserParams_last->width_01mm        = (uint16_t)(dstBuffer[LASER_PARAMS_LAST_8+6] << 8 | dstBuffer[LASER_PARAMS_LAST_8+7]);

		laserParams_last->origin_y0_01mm = (uint16_t)(dstBuffer[LASER_ORIGIN_LAST_4+0] << 8 | dstBuffer[LASER_ORIGIN_LAST_4+1]);
		laserParams_last->origin_x0_01mm = (uint16_t)(dstBuffer[LASER_ORIGIN_LAST_4+2] << 8 | dstBuffer[LASER_ORIGIN_LAST_4+3]);

		laserParams_last->alpha_cut_en = (uint8_t)(dstBuffer[LASER_PARAMS_ALPHA_CUT_EN_LAST_1]);
		laserParams_last->laser_console_en = (uint8_t)(dstBuffer[LASER_CONSOLE_LAST_1]);

		laserParams_last->slot = 0;

	}
	return status;
}


enum eepromStatus_t saveResolutionRadiusToEEPROM(uint16_t* resolution, uint16_t* radius_01mm, uint16_t* circumference_01mm){
	enum eepromStatus_t status = EEPROM_SUCCESS;
	{
		uint8_t srcBuffer[4];
		uint8_t nr = sizeof(srcBuffer)/sizeof(srcBuffer[0]);

		srcBuffer[0] = (uint8_t) (*resolution >> 8);
		srcBuffer[1] = (uint8_t) (*resolution);
		srcBuffer[2] = (uint8_t) (*radius_01mm >> 8);
		srcBuffer[3] = (uint8_t) (*radius_01mm);

		status = bytesWriteToEEPROM((uint8_t)RES_RAD_4, srcBuffer, nr);
	}

	{
		uint8_t srcBuffer[2];
		uint8_t nr = sizeof(srcBuffer)/sizeof(srcBuffer[0]);

		srcBuffer[0] = (uint8_t) (*circumference_01mm >> 8);
		srcBuffer[1] = (uint8_t) (*circumference_01mm);
		status = bytesWriteToEEPROM((uint8_t)CIRCUMFERENCE_2, srcBuffer, nr);
	}

	return status;
}

enum eepromStatus_t saveLaserConsoleActiveToEEPROM(uint8_t* laserConsoleActive){
	uint8_t srcBuffer[1];
	uint8_t nr = sizeof(srcBuffer)/sizeof(srcBuffer[0]);

	srcBuffer[0] = (uint8_t) (*laserConsoleActive);

	enum eepromStatus_t status = bytesWriteToEEPROM((uint8_t)LASER_CONSOLE_LAST_1, srcBuffer, nr);

	return status;
}

enum eepromStatus_t saveSetLengthToEEPROM(uint32_t* setLength){
	enum eepromStatus_t status = EEPROM_SUCCESS;
	uint8_t srcBuffer[4];
	uint8_t nr = sizeof(srcBuffer)/sizeof(srcBuffer[0]);

	srcBuffer[0] = (uint8_t) (*setLength >> 24);
	srcBuffer[1] = (uint8_t) (*setLength >> 16);
	srcBuffer[2] = (uint8_t) (*setLength >> 8);
	srcBuffer[3] = (uint8_t) (*setLength >> 0);

	status = bytesWriteToEEPROM((uint8_t)SET_LEN_4, srcBuffer, nr);

	return status;
}

enum eepromStatus_t saveRelayDataToEEPROM(uint32_t* duration, uint32_t* delay, uint8_t relayIdx){
	enum eepromStatus_t status = EEPROM_SUCCESS;
	uint8_t srcBuffer[8];
	uint8_t nr = sizeof(srcBuffer)/sizeof(srcBuffer[0]);

	srcBuffer[0] = (uint8_t) (*duration >> 24);
	srcBuffer[1] = (uint8_t) (*duration >> 16);
	srcBuffer[2] = (uint8_t) (*duration >> 8);
	srcBuffer[3] = (uint8_t) (*duration >> 0);
	srcBuffer[4] = (uint8_t) (*delay >> 24);
	srcBuffer[5] = (uint8_t) (*delay >> 16);
	srcBuffer[6] = (uint8_t) (*delay >> 8);
	srcBuffer[7] = (uint8_t) (*delay >> 0);

	status = bytesWriteToEEPROM((uint8_t)RLY1_DUR_4 + 8*(relayIdx-1), srcBuffer, nr);

	return status;
}

//enum eepromStatus_t saveLanguageToEEPROM(uint32_t* setLength){
//	enum eepromStatus_t status = EEPROM_SUCCESS;
//	uint8_t srcBuffer[4];
//	uint8_t nr = sizeof(srcBuffer)/sizeof(srcBuffer[0]);
//
//	srcBuffer[0] = (uint8_t) (*setLength >> 24);
//	srcBuffer[1] = (uint8_t) (*setLength >> 16);
//	srcBuffer[2] = (uint8_t) (*setLength >> 8);
//	srcBuffer[3] = (uint8_t) (*setLength >> 0);
//
//	status = bytesWriteToEEPROM((uint8_t)SET_LENGTH, srcBuffer, nr);
//
//	return status;
//}

enum eepromStatus_t saveRelaysActiveToEEPROM(uint8_t relaysActive){
	enum eepromStatus_t status = EEPROM_SUCCESS;
	uint8_t srcBuffer[1];
	uint8_t nr = sizeof(srcBuffer)/sizeof(srcBuffer[0]);

	srcBuffer[0] = (uint8_t) (relaysActive);

	status = bytesWriteToEEPROM((uint8_t)RLY_ACTIVE_1, srcBuffer, nr);

	return status;
}

enum eepromStatus_t getLaserParamsFromSlot(qPackage_laserParams_t* laserParams, uint8_t slot){
	enum eepromStatus_t status = EEPROM_SUCCESS;

	if(slot > 3){
		return EEPROM_ERR;
	}

	{
		{
			const uint8_t bytes = 8;

			uint8_t dstBuffer[bytes];
			uint8_t nr = sizeof(dstBuffer)/sizeof(dstBuffer[0]);

			uint8_t address = (uint8_t)LASER_PARAMS_LAST_8 + slot*bytes;

			status = bytesReadFromEEPROM(address, dstBuffer, nr);
			if(status == EEPROM_SUCCESS){
				//	Save resolution, radius data and entryIdx
				laserParams->angle_alpha_01deg = (uint16_t)(dstBuffer[0] << 8 | dstBuffer[1]);
				laserParams->angle_beta_01deg  = (uint16_t)(dstBuffer[2] << 8 | dstBuffer[3]);
				laserParams->feedrate          = (uint16_t)(dstBuffer[4] << 8 | dstBuffer[5]);
				laserParams->width_01mm        = (uint16_t)(dstBuffer[6] << 8 | dstBuffer[7]);

				laserParams->slot = slot;
			}
		}
		{
			const uint8_t bytes = 1;

			uint8_t dstBuffer[bytes];
			uint8_t nr = sizeof(dstBuffer)/sizeof(dstBuffer[0]);
			uint8_t address = (uint8_t)LASER_PARAMS_ALPHA_CUT_EN_LAST_1 + slot*bytes;
			status = bytesReadFromEEPROM(address, dstBuffer, nr);
			if(status == EEPROM_SUCCESS){
				//	Save alpha cut en (else only beta, single cut)
				laserParams->alpha_cut_en = (uint8_t)(dstBuffer[LASER_PARAMS_ALPHA_CUT_EN_LAST_1]);

			}

		}

	}

	if(status != EEPROM_SUCCESS){
		return status;
	}

	{
		const uint8_t bytes = 4;

		uint8_t dstBuffer[bytes];
		uint8_t nr = sizeof(dstBuffer)/sizeof(dstBuffer[0]);

		uint8_t address = (uint8_t)LASER_ORIGIN_LAST_4 + slot*bytes;

		status = bytesReadFromEEPROM(address, dstBuffer, nr);
		if(status == EEPROM_SUCCESS){
			//	Save resolution, radius data and entryIdx

			laserParams->origin_y0_01mm = (uint16_t)(dstBuffer[0] << 8 | dstBuffer[1]);
			laserParams->origin_x0_01mm = (uint16_t)(dstBuffer[2] << 8 | dstBuffer[3]);

		}
	}
	return status;
}

enum eepromStatus_t saveLaserParamsToEEPROM(qPackage_laserParams_t laserParams){
	enum eepromStatus_t status = EEPROM_SUCCESS;

	if(laserParams.slot > 3){
		return EEPROM_ERR;
	}

	{
		// 4 members, each 2 bytes = 8 bytes
		const uint8_t bytes = 8;
		uint8_t srcBuffer[bytes];
		uint8_t nr = sizeof(srcBuffer)/sizeof(srcBuffer[0]);

		srcBuffer[0] = (uint8_t) (laserParams.angle_alpha_01deg >> 8);
		srcBuffer[1] = (uint8_t) (laserParams.angle_alpha_01deg >> 0);
		srcBuffer[2] = (uint8_t) (laserParams.angle_beta_01deg >> 8);
		srcBuffer[3] = (uint8_t) (laserParams.angle_beta_01deg >> 0);
		srcBuffer[4] = (uint8_t) (laserParams.feedrate >> 8);
		srcBuffer[5] = (uint8_t) (laserParams.feedrate >> 0);
		srcBuffer[6] = (uint8_t) (laserParams.width_01mm >> 8);
		srcBuffer[7] = (uint8_t) (laserParams.width_01mm >> 0);

		uint8_t address = (uint8_t)LASER_PARAMS_LAST_8 + laserParams.slot*bytes;
		status = bytesWriteToEEPROM(address, srcBuffer, nr);
	}

	if(status != EEPROM_SUCCESS){
		return status;
	}

	{
		// 1 members, each 1 bytes = 1 bytes
		const uint8_t bytes = 1;
		uint8_t srcBuffer[bytes];
		uint8_t nr = sizeof(srcBuffer)/sizeof(srcBuffer[0]);

		srcBuffer[0] = (uint8_t) (laserParams.alpha_cut_en);

		uint8_t address = (uint8_t)LASER_PARAMS_ALPHA_CUT_EN_LAST_1 + laserParams.slot*bytes;
		status = bytesWriteToEEPROM(address, srcBuffer, nr);
	}

	{
		// 2 members, each 2 bytes = 4 bytes
		const uint8_t bytes = 4;
		uint8_t srcBuffer[bytes];
		uint8_t nr = sizeof(srcBuffer)/sizeof(srcBuffer[0]);

		srcBuffer[0] = (uint8_t) (laserParams.origin_y0_01mm >> 8);
		srcBuffer[1] = (uint8_t) (laserParams.origin_y0_01mm >> 0);
		srcBuffer[2] = (uint8_t) (laserParams.origin_x0_01mm >> 8);
		srcBuffer[3] = (uint8_t) (laserParams.origin_x0_01mm >> 0);

		uint8_t address = (uint8_t)LASER_ORIGIN_LAST_4 + laserParams.slot*bytes;
		status = bytesWriteToEEPROM(address, srcBuffer, nr);
	}


	return status;
}

enum eepromStatus_t saveLanguageIdxToEEPROM(uint8_t languageIdx){
	enum eepromStatus_t status = EEPROM_SUCCESS;
	uint8_t srcBuffer[1];
	uint8_t nr = sizeof(srcBuffer)/sizeof(srcBuffer[0]);

	srcBuffer[0] = (uint8_t) (languageIdx);

	status = bytesWriteToEEPROM((uint8_t)LANG_IDX_1, srcBuffer, nr);

	return status;
}


