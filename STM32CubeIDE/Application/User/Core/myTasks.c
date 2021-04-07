
/*
 * FreeRTOS tasks
 *
 */
#include "FreeRTOS.h"
#include "event_groups.h"
#include "main.h"
#include "myTasks.h"
#include "qPackages.h"
#include "eeprom.h"
#include "string.h"


/*
 * A task that waits to receive instructions about set length,
 * and whether the timer should be active and thus counting encoder pulses.
 * It generates update event when immediate cut instruction is received.
 *
 * This task should be higher priority than GUI task, so it pre-empts the GUI task when data is available.
 *
 * Package structure contains: uint8_t isActive; uint8_t immCut; uint32_t length;
 */
void encoderControlTask(void *pvParameters){
	extern QueueHandle_t qhGUItoEncoderControl;
	extern QueueHandle_t qhEncoderControlToReport;
	extern QueueHandle_t qhStatusReport;

	qPackage_encoderControl_t receivedSettings;
	qPackage_statusReport_t packageStatusReport;
	BaseType_t xStatus;

	for(;;){
		xStatus = xQueueReceive(qhGUItoEncoderControl, &receivedSettings, portMAX_DELAY); // will stay in block mode indefinetely
		if(xStatus == errQUEUE_EMPTY){
			//TODO: report a problem
			while(1);
		}
		if(receivedSettings.isActive){
			TIM3 -> CR1 |= TIM_CR1_CEN_Msk;
		}else{
			TIM3 -> CR1 &= ~TIM_CR1_CEN_Msk;
		}

		uint32_t newArr = (uint32_t)(0.5+(float)receivedSettings.resolution*receivedSettings.length_01mm/(2*3.1415926*receivedSettings.radius_01mm));

		if(newArr > 0xFFFF ){
			TIM3 -> ARR = (uint32_t) 0xFFFF;
			packageStatusReport.statusId = SET_LENGTH_TRIMMED;
		}else{
			TIM3 -> ARR = (uint32_t) newArr;
			packageStatusReport.statusId = SET_LENGTH_VALID;
		}

		// Issue a Update Event if new ARR is smaller than CNT
		if(TIM3 -> ARR < TIM3 -> CNT){
			TIM3->EGR = 0b1 << TIM_EGR_UG_Pos;
		}

		// Report back if set length was updated successfully
		packageStatusReport.data = (uint32_t)((float)TIM3->ARR*2*3.141592*receivedSettings.radius_01mm/receivedSettings.resolution+0.5);
		xStatus = xQueueSend(qhStatusReport, &packageStatusReport, pdMS_TO_TICKS(10)); // report actual set length
		if(xStatus == errQUEUE_FULL){
			//TODO: report a problem
			while(1);
		}

		// Forward new settings to reporting task
		xStatus = xQueueSend(qhEncoderControlToReport, &receivedSettings, pdMS_TO_TICKS(10)); // forward settings to reportTask
		if(xStatus == errQUEUE_FULL){
			//TODO: report a problem
			while(1);
		}

	}

}
/*
 * Reports current length and amount
 * */
void reportTask(void *pvParameters){
	extern QueueHandle_t qhEncoderControlToReport;
	extern QueueHandle_t qhReportToTouchGFX;

	qPackage_encoderControl_t receivedSettings;
	qPackage_report_t packageToSend;
	BaseType_t xStatus;
	uint32_t resolution; //read from EEPROM
	uint32_t radius_01mm;//read from EEPROM

	//	Wait to receive settings from EEPROM
	xStatus = xQueueReceive(qhEncoderControlToReport, &receivedSettings, portMAX_DELAY);
	if(xStatus == pdPASS){
		resolution = receivedSettings.resolution;
		radius_01mm = receivedSettings.radius_01mm;
	}

	for(;;){
		xStatus = xQueueReceive(qhEncoderControlToReport, &receivedSettings, 0);
		if(xStatus == pdPASS){
			resolution = receivedSettings.resolution;
			radius_01mm = receivedSettings.radius_01mm;
		}


		packageToSend.currLength_01mm = (uint32_t)((float)TIM3->CNT*2*3.141592*radius_01mm/resolution+0.5);
		packageToSend.amount = TIM5->CNT;

		xStatus = xQueueOverwrite(qhReportToTouchGFX, &packageToSend);
		if(xStatus == errQUEUE_FULL){
			//TODO: report a problem, this always return true anyway
			while(1);
		}

		vTaskDelay(pdMS_TO_TICKS(1000/60));
	}

}

/*
 * 	A task that runs when package with relay settings is received. *
 *
 * */
void relaySetupTask(void *pvParameters){
	BaseType_t xStatus;
	enum statusId_t statusId;

	for(;;){
		extern QueueHandle_t qhTouchGFXToRelaySetup;
		qPackage_relaySetup_t receivedSettings;
		xStatus = xQueueReceive(qhTouchGFXToRelaySetup, &receivedSettings, portMAX_DELAY); // will stay in block mode indefinetely
		if(xStatus == errQUEUE_EMPTY){
			//TODO: report a problem
			while(1);
		}
		statusId = OP_OK;
		uint32_t newArr = (uint32_t)(((uint64_t)receivedSettings.delay_ms+(uint64_t)receivedSettings.duration_ms)*100e3/(0xFFFF+1)); //100 MHz / 1000 s
		uint32_t newCC = (uint32_t)((uint64_t)receivedSettings.delay_ms*100e3/(0xFFFF+1)); //100 MHz / 1000 s
		if(newArr > 0xFFFF){
			newArr = 0xFFFF;
			statusId = RELAY_DURATION_OF;
		}
		if(newCC > 0xFFFF){
			newCC = 0xFFFF;
			statusId = RELAY_DELAY_OF;
		}
		if(newCC == 0){ // PWM mode 2: inactive as long as CNT < CCRx
			newCC = 1;
		}

		switch(receivedSettings.relayId){
		case 1:
			if(newArr == newCC){
				TIM1->CCMR1 = 0b100 << TIM_CCMR1_OC1M_Pos; // force inactive level
				statusId = RELAY_DEACTIVATED;
			}else{
				TIM1->CCMR1 = 0b111 << TIM_CCMR1_OC1M_Pos; // PWM mode 2
			}
			TIM1->ARR = newArr;
			TIM1->CCR1 = newCC;
			TIM1->EGR = 0b1 << TIM_EGR_UG_Pos;
			break;
		case 2:
			if(newArr == newCC){
				TIM12->CCMR1 = 0b100 << TIM_CCMR1_OC1M_Pos;
				statusId = RELAY_DEACTIVATED;
			}else{
				TIM12->CCMR1 = 0b111 << TIM_CCMR1_OC1M_Pos;
			}
			TIM12->ARR = newArr;
			TIM12->CCR1 = newCC;
			TIM12->EGR = 0b1 << TIM_EGR_UG_Pos;
			break;
		case 3:
			if(newArr == newCC){
				TIM8->CCMR2 = 0b100 << TIM_CCMR2_OC3M_Pos;
				statusId = RELAY_DEACTIVATED;
			}else{
				TIM8->CCMR2 = 0b111 << TIM_CCMR2_OC3M_Pos;
			}
			TIM8->ARR = newArr;
			TIM8->CCR3 = newCC;
			TIM8->EGR = 0b1 << TIM_EGR_UG_Pos;
			break;
		}
		//TODO: Report back with actual value;
		extern QueueHandle_t qhStatusReport;
		qPackage_statusReport_t packageToSend;
		packageToSend.statusId = statusId;
		xStatus = xQueueSend(qhStatusReport, &packageToSend, pdMS_TO_TICKS(10));
		if(xStatus == errQUEUE_FULL){
			//TODO: report a problem
			while(1);
		}

	}
}

/*
 * This task determines the event and executes appropriate action.
 * */
void singleEventTask(void *pvParameters){
	extern EventGroupHandle_t ehEvents;
	EventBits_t eventBits;

	for(;;){
		//	Wait until at least one event bit is set
		eventBits = xEventGroupWaitBits(ehEvents, EVENT_BITS_ALL, pdTRUE, pdFALSE, portMAX_DELAY);

		/*	RESET AMOUNT  */
		if(eventBits & EVENT_BIT_RST_AMOUNT){
			TIM5 -> CNT = 0;
		}

		/*	RESET CURRENT LENGTH*/
		if(eventBits & EVENT_BIT_RST_CURR_LEN){
			TIM3 -> CNT = 0;
		}

		/*	IMMEDIATE CUT  */
		if(eventBits & EVENT_BIT_IMM_CUT){
			TIM3 -> EGR = TIM_EGR_UG_Msk;
			TIM5 -> CNT -=1; // cutting a new piece increments amount counter
		}

		/*	ACTIVATE RELAYS	 */
		if(eventBits & EVENT_BIT_RELAYS_ACTIVATE){
			enableRelayTimers();
		}

		/*	DEACTIVATE RELAYS  */
		if(eventBits & EVENT_BIT_RELAYS_DEACTIVATE){
			disableRelayTimers();
		}

		/*	LOAD SETTINGS  */
		if(eventBits & EVENT_BIT_LOAD_SETTINGS){
			extern QueueHandle_t qhSettingsToGUI;
			qPackage_settings_t settingsToSend;
			BaseType_t xStatus;
			enum eepromStatus_t status;

			//	Prepare array of pointers to locations for getSettingsFromEEPROM() to fill in
			uint32_t *relayData[] = {
							&settingsToSend.relay1.duration_ms, &settingsToSend.relay1.delay_ms,
							&settingsToSend.relay2.duration_ms, &settingsToSend.relay2.delay_ms,
							&settingsToSend.relay3.duration_ms, &settingsToSend.relay3.delay_ms,
			};
			//	Get settings
			status = getSettingsFromEEPROM(
							&settingsToSend.encoderControl.resolution,
							&settingsToSend.encoderControl.radius_01mm,
							&settingsToSend.encoderControl.length_01mm,
							relayData,
							&settingsToSend.languageIdx,
							&settingsToSend.relaysActive,
							&settingsToSend.brightness);
			if(status == EEPROM_SUCCESS){
				settingsToSend.settingsMask = SETTINGS_ALL_Bit;
			}else{
				settingsToSend.settingsMask = SETTINGS_FAIL_Bit;
			}
			// Only the following line can send to this queue, so it should never be full as it will be emptied before this function can be called again.
			xStatus = xQueueSend(qhSettingsToGUI, &settingsToSend, pdMS_TO_TICKS(0));

			if(xStatus == errQUEUE_FULL){
				//TODO: report a problem
				while(1);
			}

		}
	}

}


/*
 * This task waits for a message and then writes data to EEPROM
 * */
void writeSettingsTask(void *pvParamaters){
	extern QueueHandle_t qhGUItoWriteSettings;
	qPackage_settings_t receivedSettings;
	BaseType_t xStatus;

	for(;;){
		//	Wait for new settings to arrive
		xStatus = xQueueReceive(qhGUItoWriteSettings, &receivedSettings, portMAX_DELAY); // will stay in block mode indefinetely
		if(xStatus != pdTRUE){
			//TODO: report a problem
			while(1);
		}

		enum eepromStatus_t status = EEPROM_SUCCESS;
//		if(receivedSettings.settingsMask & SETTINGS_ALL_Bit){
//			status = saveResolutionRadiusToEEPROM(&receivedSettings.encoderControl.resolution, &receivedSettings.encoderControl.radius_01mm);
//			status = saveSetLengthToEEPROM(&receivedSettings.encoderControl.length_01mm);
//			status = saveRelayDataToEEPROM(&receivedSettings.relay1.duration_ms, &receivedSettings.relay1.delay_ms, 1);
//			status = saveRelayDataToEEPROM(&receivedSettings.relay2.duration_ms, &receivedSettings.relay2.delay_ms, 2);
//			status = saveRelayDataToEEPROM(&receivedSettings.relay3.duration_ms, &receivedSettings.relay3.delay_ms, 3);
//			status = saveLanguageIdxToEEPROM(&receivedSettings.languageIdx);
//			status = saveRelaysActiveToEEPROM(&receivedSettings.relaysActive);
//		}else

		if(status == EEPROM_SUCCESS && receivedSettings.settingsMask & SETTINGS_RES_RAD_Bit){
			status = saveResolutionRadiusToEEPROM(&receivedSettings.encoderControl.resolution, &receivedSettings.encoderControl.radius_01mm);
		}
		if(status == EEPROM_SUCCESS && receivedSettings.settingsMask & SETTINGS_LENGTH_Bit){
			status = saveSetLengthToEEPROM(&receivedSettings.encoderControl.length_01mm);
		}
		if(status == EEPROM_SUCCESS && receivedSettings.settingsMask & SETTINGS_RELAY1_Bit){
			status = saveRelayDataToEEPROM(&receivedSettings.relay1.duration_ms, &receivedSettings.relay1.delay_ms, 1);
		}
		if(status == EEPROM_SUCCESS && receivedSettings.settingsMask & SETTINGS_RELAY2_Bit){
			status = saveRelayDataToEEPROM(&receivedSettings.relay2.duration_ms, &receivedSettings.relay2.delay_ms, 2);
		}
		if(status == EEPROM_SUCCESS && receivedSettings.settingsMask & SETTINGS_RELAY3_Bit){
			status = saveRelayDataToEEPROM(&receivedSettings.relay3.duration_ms, &receivedSettings.relay3.delay_ms, 3);
		}
		if(status == EEPROM_SUCCESS && receivedSettings.settingsMask & SETTINGS_LANG_IDX_Bit){
			status = saveLanguageIdxToEEPROM(receivedSettings.languageIdx);
		}
		if(status == EEPROM_SUCCESS && receivedSettings.settingsMask & SETTINGS_RELAY_ACT_Bit){
			status = saveRelaysActiveToEEPROM(receivedSettings.relaysActive);
		}

		if(status != EEPROM_SUCCESS ){
			qPackage_statusReport_t packageToSend;
			extern QueueHandle_t qhStatusReport;
			extern const char* eepromStatus_strings[];

			packageToSend.statusId = SETTINGS_SAVE_ERR;
			packageToSend.data = status;
			strcpy(packageToSend.message, eepromStatus_strings[status]);

			xStatus = xQueueSend(qhStatusReport, &packageToSend, pdMS_TO_TICKS(0));
			if(xStatus != pdTRUE){
				//TODO: report a problem
				while(1);
			}

		}
	}
}



