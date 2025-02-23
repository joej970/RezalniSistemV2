/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
enum statusId_t{
	OP_OK,
	SET_LENGTH_TRIMMED,
	SET_LENGTH_VALID,
	RELAY_DELAY_OF,
	RELAY_DURATION_OF,
	RELAY_DEACTIVATED,
	RELAY_ACTIVATED,
	SET_LENGTH_OF,
	SETTINGS_LOAD_SUCCESS,
	SETTINGS_LOAD_ERR,
	SETTINGS_SAVE_ERR,
	UART_RX_ERR,
	UART_TX_NOT_OKED,
	UART_TX_ERR,
	UART_RX_SUCCESS,
	UART_TX_SUCCESS,
	OTHER_ERR
};

enum grblConn_t{
	NOT_CONNECTED,
	PENDING,
	CONNECTED
};

//enum eepromStatus_t{
//	EEPROM_SUCCESS,
//	EEPROM_TIMEOUT_1,
//	EEPROM_TIMEOUT_2,
//	EEPROM_TIMEOUT_3,
//	EEPROM_TIMEOUT_4,
//	EEPROM_BUSY,
//	EEPROM_TXFULL,
//	EEPROM_ERR
//};

//const char* eepromStatus_strings[]  = {
//				[EEPROM_SUCCESS] = "EEPROM_SUCCESS",
//				[EEPROM_TIMEOUT_2] = "EEPROM_TIMEOUT_2",
//				[EEPROM_TIMEOUT_1] = "EEPROM_TIMEOUT_3",
//				[EEPROM_TIMEOUT_1] = "EEPROM_TIMEOUT_3",
//				[EEPROM_TIMEOUT_4] = "EEPROM_TIMEOUT_4",
//				[EEPROM_BUSY] = "EEPROM_BUSY",
//				[EEPROM_TXFULL] = "EEPROM_TXFULL",
//				[EEPROM_ERR] = "EEPROM_ERR"
//};



//enum eepromMemoryMap_t{
////	RES_ENTRYIDX_RADIUS_ENTRYVALID = 0,
//	RESOLUTION_RADIUS	= 0,
//	SET_LENGTH 	= 4,
//	RLY1_DUR 	= 8,
//	RLY1_DEL	= 12,
//	RLY2_DUR 	= 16,
//	RLY2_DEL 	= 20,
//	RLY3_DUR 	= 24,
//	RLY3_DEL 	= 28,
//	OFFSET		= 32
//};


/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void disableRelayTimers(void);
void enableRelayTimers(void);
void reportErrorToPopUp(enum statusId_t statusId, char* description, uint32_t data);
//enum eepromStatus_t byteWriteToEEPROM(uint8_t dataAddr, uint8_t data);
//enum eepromStatus_t bytesWriteToEEPROM(uint8_t dataAddr, uint8_t *srcBuffer, uint8_t nr);
//enum eepromStatus_t bytesReadFromEEPROM(uint8_t dataAddr, uint8_t *dstBuffer, uint8_t nr);
//enum eepromStatus_t getResolutionRadiusFromEEPROM(uint16_t*, uint16_t*);
//enum eepromStatus_t getSettingsFromEEPROM(uint16_t* resolution, uint16_t* radius_01mm, uint32_t* setLength_01mm, uint32_t *relayData[6]);
//enum eepromStatus_t saveRelayDataToEEPROM(uint32_t* duration, uint32_t* delay, uint8_t relayIdx);
//enum eepromStatus_t saveSetLengthToEEPROM(uint32_t* setLength);
//enum eepromStatus_t saveResolutionRadiusToEEPROM(uint16_t* resolution, uint16_t* radius_01mm);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_BL_CTRL_Pin GPIO_PIN_3
#define LCD_BL_CTRL_GPIO_Port GPIOK
#define ERROR_LED_Pin GPIO_PIN_1
#define ERROR_LED_GPIO_Port GPIOI
#define LCD_DISP_Pin GPIO_PIN_12
#define LCD_DISP_GPIO_Port GPIOI
#define RLY1_Pin GPIO_PIN_8
#define RLY1_GPIO_Port GPIOA
#define ENC2_Pin GPIO_PIN_7
#define ENC2_GPIO_Port GPIOC
#define ENC1_Pin GPIO_PIN_6
#define ENC1_GPIO_Port GPIOC
#define UART7_TX_Pin GPIO_PIN_7
#define UART7_TX_GPIO_Port GPIOF
#define UART7_RX_Pin GPIO_PIN_6
#define UART7_RX_GPIO_Port GPIOF
#define RLY2_Pin GPIO_PIN_14
#define RLY2_GPIO_Port GPIOB
#define RLY3_Pin GPIO_PIN_15
#define RLY3_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
//  Event masking bits
#define EVENT_BIT_IMM_CUT 			(1UL << 0UL)
#define EVENT_BIT_RST_AMOUNT		(1UL << 1UL)
#define EVENT_BIT_LOAD_SETTINGS		(1UL << 2UL)
#define EVENT_BIT_RELAYS_ACTIVATE	(1UL << 3UL)
#define EVENT_BIT_RELAYS_DEACTIVATE	(1UL << 4UL)
#define EVENT_BIT_RST_CURR_LEN		(1UL << 5UL)
#define EVENT_BIT_LOAD_LASER_PARAMS_SLOT_1 (1UL << 6UL)
#define EVENT_BIT_LOAD_LASER_PARAMS_SLOT_2 (1UL << 7UL)
#define EVENT_BIT_LOAD_LASER_PARAMS_SLOT_3 (1UL << 8UL)
#define EVENT_BIT_ENABLE_LASER_CUTTING (1UL << 11UL)
#define EVENT_BIT_DISABLE_LASER_CUTTING (1UL << 12UL)

#define EVENT_BITS_ALL			(EVENT_BIT_DISABLE_LASER_CUTTING | EVENT_BIT_ENABLE_LASER_CUTTING | EVENT_BIT_LOAD_LASER_PARAMS_SLOT_3 | EVENT_BIT_LOAD_LASER_PARAMS_SLOT_2 | EVENT_BIT_LOAD_LASER_PARAMS_SLOT_1 | EVENT_BIT_IMM_CUT | EVENT_BIT_RST_AMOUNT | EVENT_BIT_LOAD_SETTINGS | EVENT_BIT_RELAYS_ACTIVATE | EVENT_BIT_RELAYS_DEACTIVATE | EVENT_BIT_RST_CURR_LEN)

// ehEventsGRBL
#define EVENT_GRBL_BIT_UPLOAD_GCODE_TO_GRBL_AND_START (1UL << 20UL)
#define EVENT_GRBL_BIT_UPLOAD_GCODE_TO_GRBL (1UL << 21UL)
#define EVENT_GRBL_BIT_SEND_HOME (1UL << 22UL)
#define EVENT_GRBL_BIT_SEND_TO_ORIGIN (1UL << 23UL)
#define EVENT_GRBL_BIT_INITIATE_GRBL_CONTROLLER (1UL << 24UL)
#define EVENT_GRBL_BIT_UPDATE_ORIGIN (1UL << 25UL)
#define EVENT_GRBL_BIT_UPDATE_GRBL_PARAMS (1UL << 26UL)

#define EVENT_GRBL_BITS_ALL (EVENT_GRBL_BIT_UPDATE_ORIGIN | EVENT_GRBL_BIT_SEND_TO_ORIGIN | EVENT_GRBL_BIT_SEND_HOME | EVENT_GRBL_BIT_UPDATE_GRBL_CONTROLLER | EVENT_GRBL_BIT_UPDATE_AND_START_GRBL_CONTROLLER | EVENT_GRBL_BIT_INITIATE_GRBL_CONTROLLER)

////  Memory organisation
//#define RESOLUTION_Pos	0U
//#define ENTRY_IDX_Pos 	14U
//#define RADIUS_Pos		16U
//#define ENTRY_VALID_Pos	31U
//
//#define RESOLUTION_Msk	(0x3FFFUL << RESOLUTION_Pos)
//#define ENTRY_IDX_Msk  	(3UL << ENTRY_IDX_Pos)
//#define RADIUS_Msk		(0x7FFFUL << RADIUS_Pos)
//#define ENTRY_VALID_Msk (1UL << ENTRY_VALID_Pos)

//  Settings bits
#define SETTINGS_FAIL_Bit		(0UL)
#define SETTINGS_NONE_Bit		(1UL << 0U)
#define SETTINGS_ALL_Bit 		(1UL << 1U)
#define SETTINGS_RES_RAD_Bit	(1UL << 2U)
#define SETTINGS_LENGTH_Bit		(1UL << 3U)
#define SETTINGS_RELAY1_Bit		(1UL << 4U)
#define SETTINGS_RELAY2_Bit		(1UL << 5U)
#define SETTINGS_RELAY3_Bit		(1UL << 6U)
#define SETTINGS_LANG_IDX_Bit	(1UL << 7U)
#define SETTINGS_RELAY_ACT_Bit	(1UL << 8U)
#define SETTINGS_BRIGHTNESS_Bit	(1UL << 9U)
#define SETTINGS_LASER_PARAMS_Bit	(1UL << 10U)
//#define SETTINGS_BRIGHTNESS_Bit	(1UL << 11U)
//#define SETTINGS_BRIGHTNESS_Bit	(1UL << 12U)
//#define SETTINGS_BRIGHTNESS_Bit	(1UL << 13U)

#define UART_RX_GLOBAL_BUFFER_SIZE 100

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
