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
	SET_LENGTH_OF
};

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
enum eepromStatus_t byteWriteToEEPROM(uint8_t dataAddr, uint8_t data);
enum eepromStatus_t bytesWriteToEEPROM(uint8_t dataAddr, uint8_t *srcBuffer, uint8_t nr);
enum eepromStatus_t bytesReadFromEEPROM(uint8_t dataAddr, uint8_t *dstBuffer, uint8_t nr);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_BL_CTRL_Pin GPIO_PIN_3
#define LCD_BL_CTRL_GPIO_Port GPIOK
#define LCD_DISP_Pin GPIO_PIN_12
#define LCD_DISP_GPIO_Port GPIOI
#define RLY1_Pin GPIO_PIN_8
#define RLY1_GPIO_Port GPIOA
#define ENC2_Pin GPIO_PIN_7
#define ENC2_GPIO_Port GPIOC
#define ENC1_Pin GPIO_PIN_6
#define ENC1_GPIO_Port GPIOC
#define TRIGGER_OUT_Pin GPIO_PIN_7
#define TRIGGER_OUT_GPIO_Port GPIOF
#define RLY2_Pin GPIO_PIN_14
#define RLY2_GPIO_Port GPIOB
#define RLY3_Pin GPIO_PIN_15
#define RLY3_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define EVENT_BIT_IMM_CUT 		(0b1UL << 0UL)
#define EVENT_BIT_RST_AMMOUNT	(0b1UL << 1UL)
#define EVENT_BITS_ALL			(EVENT_BIT_IMM_CUT | EVENT_BIT_RST_AMMOUNT)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
