
/*
 * FreeRTOS tasks
 *
 */

#include <stdio.h>

#include "FreeRTOS.h"
#include "event_groups.h"
#include "main.h"
#include "myTasks.h"
#include "qPackages.h"
#include "eeprom.h"
#include "string.h"
#include "queue.h"
#include "GRBL_control.h"

extern QueueHandle_t qhTouchGFXToGRBLControl;
extern QueueHandle_t qhEncoderControlToReport;
extern QueueHandle_t qhReportToTouchGFX;
extern QueueHandle_t qhTouchGFXToRelaySetup;
extern EventGroupHandle_t ehEvents;
extern QueueHandle_t qhGUItoEncoderControl;
extern QueueHandle_t qhSettingsToGUI;
extern QueueHandle_t qhStatusReport;
extern const char* eepromStatus_strings[];
extern QueueHandle_t qhGUItoWriteSettings;
//extern EventGroupHandle_t ehEventsGRBL;
extern enum grblConn_t grblConnectionStatus;
extern bool grblInstructionsUploaded;
extern UART_HandleTypeDef huart7;
extern char* pUARTRxGlobalBuffer;

/*
 * This task waits to receive instructions about set length,
 * and whether the timer should be active and thus counting encoder pulses.
 *
 * This task should be higher priority than GUI task, so it pre-empts the GUI task when data is available.
 *
 * Package structure contains: uint8_t isActive; uint8_t immCut; uint32_t length;
 */
void encoderControlTask(void *pvParameters){

	qPackage_encoderControl_t receivedSettings;
	qPackage_statusReport_t packageStatusReport;
	BaseType_t xStatus;

	for(;;){
		xStatus = xQueueReceive(qhGUItoEncoderControl, &receivedSettings, portMAX_DELAY); // will stay in block mode indefinetely
		if(xStatus == errQUEUE_EMPTY){
			//TODO: report a problem
			while(1){
				HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
			};
		}
		if(receivedSettings.isActive){
			TIM3 -> CR1 |= TIM_CR1_CEN_Msk;
			// if GRBL controller is connected, enable GRBL control by enabling global TIM3 interrupt
			if(grblConnectionStatus == CONNECTED){
				enableGRBLcontrol();
			}
		}else{
			TIM3 -> CR1 &= ~TIM_CR1_CEN_Msk;
			disableGRBLcontrol();
		}

		uint32_t newArr = (uint32_t)(0.5+(float)receivedSettings.resolution*receivedSettings.length_01mm/(2*3.1415926*receivedSettings.radius_01mm));

		if(newArr > 0xFFFF ){
			TIM3 -> ARR = (uint32_t) 0xFFFF;
			packageStatusReport.statusId = SET_LENGTH_TRIMMED;
		}else{
			TIM3 -> ARR = (uint32_t) newArr;
			packageStatusReport.statusId = SET_LENGTH_VALID;
		}

		// Set up capture compare on CH1 to construct and send message.
		TIM3->CCR1 = (uint32_t)(0.8*(float)TIM3->ARR);

		// Issue an Update Event if new ARR is smaller than CNT
		if(TIM3 -> ARR < TIM3 -> CNT){
			TIM3->EGR = 0b1 << TIM_EGR_UG_Pos;
		}

		// Report back if set length was updated successfully
		packageStatusReport.data = (uint32_t)((float)TIM3->ARR*2*3.141592*receivedSettings.radius_01mm/receivedSettings.resolution+0.5);
		xStatus = xQueueSend(qhStatusReport, &packageStatusReport, pdMS_TO_TICKS(10)); // report actual set length
		if(xStatus == errQUEUE_FULL){
			//TODO: report a problem
			while(1){
				HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
			};
		}

		// Forward new settings to reporting task
		xStatus = xQueueSend(qhEncoderControlToReport, &receivedSettings, pdMS_TO_TICKS(10)); // forward settings to reportTask
		if(xStatus == errQUEUE_FULL){
			//TODO: report a problem
			while(1){
				HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
			};
		}

	}

}

/*
 * GRBL communication task. Should listen for updates to cutting parameters.
 * And should construct message and send it to GRBL controller,
 * waiting for "ok" response after every line.
 * Sets a global variable to indicate commands were transfered
 * so that resume char "~" can be sent from TIM1_CC interrupt routine.
 * */

void grblCommunicationTask(void *pvParameters){

	uint32_t eventBits;

	uint16_t alpha_01deg;
	uint16_t beta_01deg  ;
	uint16_t width_01mm ;
	uint16_t feedrate    ;
	uint16_t origin_y0_01mm;
	uint16_t origin_x0_01mm;
	double velocity_mm_ms;

	bool paramsConfigured = false;

	HAL_UARTEx_ReceiveToIdle_IT(&huart7, (uint8_t*)pUARTRxGlobalBuffer, UART_RX_GLOBAL_BUFFER_SIZE);

	initiateGrblController();

	for(;;){

		qPackage_laserParams_t laserParameters;
		BaseType_t xStatus = xQueueReceive(qhTouchGFXToGRBLControl, &laserParameters, portMAX_DELAY);
		if(xStatus == pdPASS){
			eventBits = laserParameters.eventMask;


//			eventBits = xEventGroupWaitBits(ehEventsGRBL, EVENT_GRBL_BITS_ALL, pdTRUE, pdFALSE, portMAX_DELAY);
			printf("New event has arrived to qhTouchGFXToGRBLControl queue: %lx !\n", eventBits);

			/*	RE-INITIATE GRBL CONTROLLER  */
			if(eventBits & EVENT_GRBL_BIT_INITIATE_GRBL_CONTROLLER){
				initiateGrblController();
			}
			/*	RE-INITIATE GRBL CONTROLLER  */
			if(eventBits & EVENT_GRBL_BIT_UPDATE_GRBL_PARAMS){
				alpha_01deg    = laserParameters.angle_alpha_01deg;
				beta_01deg     = laserParameters.angle_beta_01deg;
				width_01mm     = laserParameters.feedrate;
				feedrate       = laserParameters.width_01mm;
				origin_y0_01mm = laserParameters.origin_y0_01mm;
				origin_x0_01mm = laserParameters.origin_x0_01mm;
				paramsConfigured = true;
			}

			/*	SEND TO HOME */
			if(eventBits & EVENT_GRBL_BIT_SEND_HOME){
				sendToHomeGRBL();
			}

			/*	SEND TO ORIGIN */
			if(eventBits & EVENT_GRBL_BIT_SEND_TO_ORIGIN){
				origin_y0_01mm = laserParameters.origin_y0_01mm;
				origin_x0_01mm = laserParameters.origin_x0_01mm;
//				struct coords_t point_0 = {x=origin_x0_01mm/10.0, y=origin_y0_01mm/10.0};
				sendToOriginGRBL(origin_x0_01mm/10.0, origin_y0_01mm/10.0);
			}

			/*	UPDATE ORIGIN */
			if(eventBits & EVENT_GRBL_BIT_UPDATE_ORIGIN){
				origin_y0_01mm = laserParameters.origin_y0_01mm;
				origin_x0_01mm = laserParameters.origin_x0_01mm;
//				struct coords_t point_0 = {origin_x0_01mm/10.0, origin_y0_01mm/10.0};
				updateOriginGRBL(origin_x0_01mm/10.0, origin_y0_01mm/10.0);
			}

			/*	UPLOAD NEW COMMANDS TO GRBL CONTROLLER */
			if(eventBits & (EVENT_GRBL_BIT_UPLOAD_GCODE_TO_GRBL | EVENT_GRBL_BIT_UPLOAD_GCODE_TO_GRBL_AND_START)){

//				alpha_01deg    = laserParameters.angle_alpha_01deg;
//				beta_01deg     = laserParameters.angle_beta_01deg;
//				width_01mm     = laserParameters.feedrate;
//				feedrate       = laserParameters.width_01mm;
//				origin_y0_01mm = laserParameters.origin_y0_01mm;
//				origin_x0_01mm = laserParameters.origin_x0_01mm;
//				paramsConfigured = true;

				// if parameters were updated at least once
				if(!paramsConfigured){

					reportErrorToPopUp(SETTINGS_LOAD_ERR ,"Laser params NaN!", 0);

				}

				velocity_mm_ms = 0.0;
				int16_t status = assembleAndSendCommandsToGRBL_impl(alpha_01deg, beta_01deg, width_01mm, feedrate, velocity_mm_ms, origin_x0_01mm, origin_y0_01mm);

	//			bool sendStart = eventBits & EVENT_BIT_UPDATE_AND_START_GRBL_CONTROLLER ? true : false;
				if(status == 0){
					if(eventBits & EVENT_GRBL_BIT_UPLOAD_GCODE_TO_GRBL_AND_START){
						sendContinueToGRBL();
					}
				}else{
	//				sprintf("Fault while sending G-code.\n");
					reportErrorToPopUp(OTHER_ERR ,"Fault while sending G-code.", 0);

	//				while(1);
				}

			}
		}

	}

}




/*
 * Reports current length and amount
 * */
void reportTask(void *pvParameters){


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
			while(1){
				HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
			};
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

		qPackage_relaySetup_t receivedSettings;
		xStatus = xQueueReceive(qhTouchGFXToRelaySetup, &receivedSettings, portMAX_DELAY); // will stay in block mode indefinetely
		if(xStatus == errQUEUE_EMPTY){
			//TODO: report a problem
			while(1){
				HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
			};
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

		qPackage_statusReport_t packageToSend;
		packageToSend.statusId = statusId;
		xStatus = xQueueSend(qhStatusReport, &packageToSend, pdMS_TO_TICKS(10));
		if(xStatus == errQUEUE_FULL){
			//TODO: report a problem
			while(1){
				HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
			};
		}

	}
}

/*
 * This task determines the event and executes appropriate action.
 * */
void singleEventTask(void *pvParameters){

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

			if(grblConnectionStatus == CONNECTED){
				grblInstructionsUploaded = false; // this is to prevent TIM3 UE interrupt from sending "~"
//				xEventGroupSetBits(ehEventsGRBL, EVENT_GRBL_BIT_UPLOAD_GCODE_TO_GRBL_AND_START);

				qPackage_laserParams_t packageToSend;
				packageToSend.eventMask = EVENT_GRBL_BIT_UPLOAD_GCODE_TO_GRBL_AND_START;

				BaseType_t xStatus = xQueueSend(qhTouchGFXToGRBLControl, &packageToSend, 0); // should be able to put in the queue as it will be emptied immediately
				if (xStatus != pdTRUE) {
					//TODO: report a problem
					while(1){
						HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
					};
				}
			}

			TIM3 -> EGR = TIM_EGR_UG_Msk;

//			disableGRBLcontrol();
			TIM5 -> CNT -=1; // cutting a new piece increments amount counter
		}

		/* ACTIVATE LASER CUTTING */
		if(eventBits & EVENT_BIT_ENABLE_LASER_CUTTING){
//			assembleAndSendCommandsToGRBL();
			if(grblConnectionStatus == CONNECTED){
//				xEventGroupSetBits(ehEventsGRBL, EVENT_GRBL_BIT_UPDATE_GRBL_CONTROLLER);

				qPackage_laserParams_t packageToSend;

				packageToSend.eventMask = EVENT_GRBL_BIT_UPLOAD_GCODE_TO_GRBL;

				BaseType_t xStatus = xQueueSend(qhTouchGFXToGRBLControl, &packageToSend, 0); // should be able to put in the queue as it will be emptied immediately
				if (xStatus != pdTRUE) {
					//TODO: report a problem
					while(1){
						HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
					};
				}
				enableGRBLcontrol();
			}
		}
		/* DEACTIVATE LASER CUTTING */
		if(eventBits & EVENT_BIT_DISABLE_LASER_CUTTING){
			disableGRBLcontrol();
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
							&settingsToSend.brightness,
							&settingsToSend.laserParams);
			if(status == EEPROM_SUCCESS){
				settingsToSend.settingsMask = SETTINGS_ALL_Bit;
			}else{
				settingsToSend.settingsMask = SETTINGS_FAIL_Bit;
			}
			// Only the following line can send to this queue, so it should never be full as it will be emptied before this function can be called again.
			xStatus = xQueueSend(qhSettingsToGUI, &settingsToSend, pdMS_TO_TICKS(0));

			if(xStatus == errQUEUE_FULL){
				//TODO: report a problem
				while(1){
					HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
				};
			}



		}

		if((eventBits & EVENT_BIT_LOAD_LASER_PARAMS_SLOT_1) | (eventBits & EVENT_BIT_LOAD_LASER_PARAMS_SLOT_2) | (eventBits & EVENT_BIT_LOAD_LASER_PARAMS_SLOT_3)){

			qPackage_settings_t settingsToSend;
			BaseType_t xStatus;
			enum eepromStatus_t status;


			uint8_t slot;
			if (eventBits & EVENT_BIT_LOAD_LASER_PARAMS_SLOT_1) {
				slot = 1;
			} else if (eventBits & EVENT_BIT_LOAD_LASER_PARAMS_SLOT_2) {
				slot = 2;
			} else {
				slot = 3;
			};
			//	Get settings
			status = getLaserParamsFromSlot(&settingsToSend.laserParams, slot);

			if(status == EEPROM_SUCCESS){
				settingsToSend.settingsMask = SETTINGS_LASER_PARAMS_Bit;
			}else{
				settingsToSend.settingsMask = SETTINGS_FAIL_Bit;
			}

			// Only the following line can send to this queue, so it should never be full as it will be emptied before this function can be called again.
			xStatus = xQueueSend(qhSettingsToGUI, &settingsToSend, pdMS_TO_TICKS(0));

			if(xStatus == errQUEUE_FULL){
				//TODO: report a problem
				while(1){
					HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
				};
			}

		}

	}

}


/*
 * This task waits for a message and then writes data to EEPROM
 * */
void writeSettingsTask(void *pvParamaters){

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
		if(status == EEPROM_SUCCESS && receivedSettings.settingsMask & SETTINGS_LASER_PARAMS_Bit){
			status = saveLaserParamsToEEPROM(receivedSettings.laserParams);
		}

		if(status != EEPROM_SUCCESS ){
			qPackage_statusReport_t packageToSend;


			packageToSend.statusId = SETTINGS_SAVE_ERR;
			packageToSend.data = status;
			strcpy(packageToSend.message, eepromStatus_strings[status]);

			xStatus = xQueueSend(qhStatusReport, &packageToSend, pdMS_TO_TICKS(0));
			if(xStatus != pdTRUE){
				//TODO: report a problem
				while(1){
					HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
				};
			}

		}
	}
}




