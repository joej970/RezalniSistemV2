
/*
 * FreeRTOS tasks
 *
 */

#include <GRBL_control.hpp>
#include <stdio.h>

#include "FreeRTOS.h"
#include "event_groups.h"
#include "main.h"
#include "myTasks.h"
#include "qPackages.h"
#include "eeprom.h"
#include "string.h"
#include "queue.h"

extern QueueHandle_t qhTouchGFXToGRBLControl;
extern QueueHandle_t qhEncoderControlToReport;
extern QueueHandle_t qhReportToTouchGFX;
extern QueueHandle_t qhTouchGFXToRelaySetup;
extern EventGroupHandle_t ehEvents;
extern EventGroupHandle_t ehEventsSoftwareTimer;
extern QueueHandle_t qhGUItoEncoderControl;
extern QueueHandle_t qhSettingsToGUI;
extern QueueHandle_t qhStatusReport;
extern const char* eepromStatus_strings[];
extern QueueHandle_t qhGUItoWriteSettings;
extern QueueHandle_t qhGRBLControlToTouchGFX;
extern QueueHandle_t qhTIM3_ISRtoTasks;
//extern EventGroupHandle_t ehEventsGRBL;
extern enum grblConn_t grblConnectionStatus;
extern bool grblInstructionsUploaded;
extern UART_HandleTypeDef huart7;
extern char* pUARTRxGlobalBuffer;

double velocity_ticks_per_1000ms_global = 0;
double velocity_mm_per_min = 0;

uint32_t resolution = 0; //read from EEPROM
uint32_t radius_01mm = 0;//read from EEPROM
uint32_t circumference_01mm = 0;//read from EEPROM


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

	enableVelocityTracking();
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
//			disableVelocityTracking();
			disableGRBLcontrol();
		}

		uint32_t newArr;
		if(receivedSettings.radius_01mm == 0){
			// use circumference
			newArr = (uint32_t)(2*(0.5+(float)receivedSettings.resolution*receivedSettings.length_01mm/(receivedSettings.circumference_01mm)));
		}else{
			// use radius
			newArr = (uint32_t)(2*(0.5+(float)receivedSettings.resolution*receivedSettings.length_01mm/(2*3.1415926*receivedSettings.radius_01mm)));
		}

		if(newArr > 0xFFFF ){
			TIM3 -> ARR = (uint32_t) 0xFFFF;
			packageStatusReport.statusId = SET_LENGTH_TRIMMED;
			packageStatusReport.data = newArr;

			char tempBufferErr[30];
			sprintf(tempBufferErr,"Length->tried: %lu", newArr);
			printf("%s\n", tempBufferErr);
			reportErrorToPopUp(SET_LENGTH_TRIMMED, tempBufferErr, newArr);
		}else{
			TIM3 -> ARR = (uint32_t) newArr;
			packageStatusReport.statusId = SET_LENGTH_VALID;
		}

		// Set up capture compare on CH1 to construct and send message.
//		TIM3->CCR1 = (uint32_t)(0.8*(float)TIM3->ARR);

		// Issue an Update Event if new ARR is smaller than CNT
		if(TIM3 -> ARR < TIM3 -> CNT){
			TIM3->EGR = 0b1 << TIM_EGR_UG_Pos;
		}

		// Report back if set length was updated successfully
		if(receivedSettings.radius_01mm == 0){
			// use circumference
			packageStatusReport.data = (uint32_t)((float)TIM3->ARR/2.0*receivedSettings.circumference_01mm/receivedSettings.resolution+0.5);
		}else{
			// use radius
			packageStatusReport.data = (uint32_t)((float)TIM3->ARR/2.0*2*3.141592*receivedSettings.radius_01mm/receivedSettings.resolution+0.5);

		}

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
	uint16_t feedrate_mm_per_min;
	uint16_t origin_y0_01mm;
	uint16_t origin_x0_01mm;
	uint8_t alphaCutActive;



//	double mm_per_tick;

	vTaskDelay( pdMS_TO_TICKS(1000) );

	bool paramsConfigured = false;

	HAL_UARTEx_ReceiveToIdle_IT(&huart7, (uint8_t*)pUARTRxGlobalBuffer, UART_RX_GLOBAL_BUFFER_SIZE);

	{
		qPackage_laserParams_t laserParameters;
		BaseType_t xStatus = xQueueReceive(qhTouchGFXToGRBLControl, &laserParameters, portMAX_DELAY);
		if(xStatus == pdPASS){
			eventBits = laserParameters.eventMask;

			/*	UPDATE PARAMETERS  */
			if(eventBits & EVENT_GRBL_BIT_UPDATE_GRBL_PARAMS){
				alpha_01deg    		= laserParameters.angle_alpha_01deg;
				beta_01deg     		= laserParameters.angle_beta_01deg;
				width_01mm     		= laserParameters.width_01mm;
				feedrate_mm_per_min = laserParameters.feedrate;
				origin_y0_01mm 		= laserParameters.origin_y0_01mm;
				origin_x0_01mm 		= laserParameters.origin_x0_01mm;
				alphaCutActive 		= laserParameters.alpha_cut_en;
				paramsConfigured 	= true;
			}
		}

	}

	if(initiateGrblController() == CONNECTED){

		qPackage_laserParams_t laserParams;
		laserParams.eventMask = EVENT_GRBL_TO_GUI_BIT_WAIT_FOR_HOMING_CONFIRM;
		BaseType_t xStatus = xQueueSend(qhGRBLControlToTouchGFX, &laserParams, pdMS_TO_TICKS(10));
		if(xStatus == errQUEUE_FULL){
			//TODO: report a problem
			while(1){
				HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
			};
		}

			// wait for homing and origin confirmation

//			reportErrorToPopUp(WAITING_FOR_HOMING_CONFIRM ,"Laser params NaN!", 0);

//		auto status = setupGRBL();
//		updateOriginGRBL(origin_x0_01mm/10.0, origin_y0_01mm/10.0);
	}

	for(;;){

		qPackage_laserParams_t laserParameters;
		BaseType_t xStatus = xQueueReceive(qhTouchGFXToGRBLControl, &laserParameters, portMAX_DELAY);
		if(xStatus == pdPASS){
			eventBits = laserParameters.eventMask;

			/* CONNECTED AND HOMING CONFIRMED/DENIED */
			if(eventBits & EVENT_GRBL_BIT_HOMING_IS_DENIED){
				disconnectGrblController();
			}

			if(eventBits & EVENT_GRBL_BIT_HOMING_IS_CONFIRMED){
				setupGRBL();
//				auto status = setupGRBL();
//				(void)status;
				updateOriginGRBL(origin_x0_01mm/10.0, origin_y0_01mm/10.0);
			}

			/*	RE-INITIATE GRBL CONTROLLER  */
			if(eventBits & EVENT_GRBL_BIT_INITIATE_GRBL_CONTROLLER){
				initiateGrblController();
				setupGRBL();
//				auto status = setupGRBL();
//				(void)status;
			}

			/*	UPDATE PARAMETERS  */
			if(eventBits & EVENT_GRBL_BIT_UPDATE_GRBL_PARAMS){
				alpha_01deg    		= laserParameters.angle_alpha_01deg;
				beta_01deg     		= laserParameters.angle_beta_01deg;
				width_01mm     		= laserParameters.width_01mm;
				feedrate_mm_per_min = laserParameters.feedrate;
				origin_y0_01mm 		= laserParameters.origin_y0_01mm;
				origin_x0_01mm 		= laserParameters.origin_x0_01mm;
				alphaCutActive 		= laserParameters.alpha_cut_en;
				paramsConfigured 	= true;
			}

			if(eventBits & (EVENT_GRBL_BIT_SEND_HOME | EVENT_GRBL_BIT_MOVE_TO_XY | EVENT_GRBL_BIT_UPDATE_ORIGIN | EVENT_GRBL_BIT_UPLOAD_GCODE_TO_GRBL | EVENT_GRBL_BIT_UPLOAD_GCODE_TO_GRBL_AND_START)){

				if(grblConnectionStatus != CONNECTED){
					reportErrorToPopUp(OTHER_ERR ,"GRBL device not connected.", 0);
					continue;
				}

				/*	SEND TO HOME */
				if(eventBits & EVENT_GRBL_BIT_SEND_HOME){
					moveGRBL(1.0, 1.0);
				}

				/*	MOVE HEAD TO */
				if(eventBits & EVENT_GRBL_BIT_MOVE_TO_XY){
					origin_y0_01mm = laserParameters.origin_y0_01mm;
					origin_x0_01mm = laserParameters.origin_x0_01mm;

					moveGRBL(origin_x0_01mm/10.0, origin_y0_01mm/10.0);
				}

				/*	UPDATE ORIGIN */
				if(eventBits & EVENT_GRBL_BIT_UPDATE_ORIGIN){
					origin_y0_01mm = laserParameters.origin_y0_01mm;
					origin_x0_01mm = laserParameters.origin_x0_01mm;

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

//					double velocity_mm_per_min;
//					if(radius_01mm == 0){
//						velocity_mm_per_min = velocity_ticks_per_1000ms_global*((1.0*60.0/10.0)/(2*3.141592))*circumference_01mm/(double)resolution;
//					}else{
//						velocity_mm_per_min = velocity_ticks_per_1000ms_global*(1.0*60.0/10.0)*radius_01mm/(double)resolution;
//					}

					// Blocking function. Will not return until all commands are successfully transfered.
					int16_t status = assembleAndSendCommandsToGRBL_impl(alpha_01deg, beta_01deg, width_01mm, feedrate_mm_per_min, velocity_mm_per_min, origin_x0_01mm, origin_y0_01mm, alphaCutActive);

					if(status == 0){
						if(eventBits & EVENT_GRBL_BIT_UPLOAD_GCODE_TO_GRBL_AND_START){
							sendContinueToGRBL();
						}
					}else{
						// Reported already inside the function. This only overwrites previous more detailed message.
//						reportErrorToPopUp(OTHER_ERR ,"Fault while sending G-code.", 0);
					}
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

	//	Wait to receive settings from EEPROM
	xStatus = xQueueReceive(qhEncoderControlToReport, &receivedSettings, portMAX_DELAY);
	if(xStatus == pdPASS){
		resolution = receivedSettings.resolution;
		radius_01mm = receivedSettings.radius_01mm;
		circumference_01mm = receivedSettings.circumference_01mm;
		if(radius_01mm == 0 && circumference_01mm == 0){
			reportErrorToPopUp(OTHER_ERR ,"rad == 0 && circ == 0.!", 0);
		}
	}

	for(;;){
		xStatus = xQueueReceive(qhEncoderControlToReport, &receivedSettings, 0);
		if(xStatus == pdPASS){
			resolution = receivedSettings.resolution;
			radius_01mm = receivedSettings.radius_01mm;
			circumference_01mm = receivedSettings.circumference_01mm;

			if(radius_01mm == 0 && circumference_01mm == 0){
				reportErrorToPopUp(OTHER_ERR ,"rad == 0 && circ == 0.!", 0);
			}

		}

		if(radius_01mm != 0){
			packageToSend.currLength_01mm = (uint32_t)((float)TIM3->CNT/2.0*(2*3.141592)*radius_01mm/resolution+0.5);
		}else{
			packageToSend.currLength_01mm = (uint32_t)((float)TIM3->CNT/2.0*circumference_01mm/resolution+0.5);
		}
		packageToSend.amount = TIM5->CNT;

		double velocity_ticks_per_1000ms_local;
		xStatus = xQueueReceive(qhTIM3_ISRtoTasks, &velocity_ticks_per_1000ms_local, 0);
		if(xStatus == pdPASS){
			velocity_ticks_per_1000ms_global = get_filtered_velocity(velocity_ticks_per_1000ms_local);
		}


		if(radius_01mm == 0){
			// 100ms
//			velocity_mm_per_min = velocity_ticks_per_1000ms_global*((10.0*60.0/10.0)/(2*3.141592))*circumference_01mm/(double)resolution;
			// 1000 ms
//			velocity_mm_per_min = velocity_ticks_per_1000ms_global*((1.0/2.0*60.0/10.0)/(2*3.141592))*circumference_01mm/(double)resolution;
			velocity_mm_per_min = (velocity_ticks_per_1000ms_global*(60.0)*((double)circumference_01mm/10.0))/(2.0*(double)resolution);
		}else{
			// 100 ms
//			velocity_mm_per_min = velocity_ticks_per_1000ms_global*(10.0*60.0/10.0)*radius_01mm/(double)resolution;
			// 1000 ms
			velocity_mm_per_min = (velocity_ticks_per_1000ms_global*(60.0)*(2*3.141592*(double)radius_01mm/10.0))/(2.0*(double)resolution);
//			velocity_mm_per_min = velocity_ticks_per_1000ms_global*(1.0/2.0*60.0/10.0)*radius_01mm/(double)resolution;
		}

		packageToSend.velocity = velocity_mm_per_min;
//		printf("Velocity mm/min: d:%d (f:%f)\n", packageToSend.velocity, velocity_mm_per_min);

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
 * Task for software timer.
 *
 *
 * */
typedef struct {
    TickType_t delay_ms;
    TickType_t duration_ms;
    GPIO_TypeDef* port;
    uint16_t pin;
} SoftwareTimerParams;


// The task function
void software_timer_task(void* pvParameters) {
    SoftwareTimerParams* params = (SoftwareTimerParams*) pvParameters;

    const TickType_t delayTicks = pdMS_TO_TICKS(params->delay_ms);
    const TickType_t durationTicks = pdMS_TO_TICKS(params->duration_ms);

    for (;;) {
        // Wait indefinitely for the event bit to be set
        xEventGroupWaitBits(ehEventsSoftwareTimer,
        					EVENT_SW_TIMER_BIT_LAUNCH,
                            pdTRUE,      // Clear bit on exit
                            pdFALSE,     // Wait for any bit
                            portMAX_DELAY);

        // Delay before setting the pin
        vTaskDelay(delayTicks);

        // Set the GPIO pin
        HAL_GPIO_WritePin(params->port, params->pin, GPIO_PIN_SET);

        // Wait while the pin stays set
        vTaskDelay(durationTicks);

        // Reset the GPIO pin
        HAL_GPIO_WritePin(params->port, params->pin, GPIO_PIN_RESET);
    }
}


TaskHandle_t softwareTimerHandle_1 = NULL;
TaskHandle_t softwareTimerHandle_2 = NULL;
TaskHandle_t softwareTimerHandle_3 = NULL;



/*
 * 	A task that runs when package with relay settings is received. *
 *
 * */
void relaySetupTask(void *pvParameters){
	BaseType_t xStatus;
	enum statusId_t statusId;
	uint8_t swTimersUsed = 0;

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

		if(newCC == 0){ // PWM mode 2: inactive as long as CNT < CCRx
			newCC = 1;
		}

		if((newArr > 0xFFFF) || (newCC > 0xFFFF)){ // use SW timers
			char task_name[20];
			TaskHandle_t* targetHandle = NULL;
			SoftwareTimerParams timerParams;
			timerParams.delay_ms = receivedSettings.delay_ms;
			timerParams.duration_ms = receivedSettings.duration_ms;

			HAL_NVIC_SetPriority(TIM4_IRQn, 5, 0);  // Priority level (1, 0)
			HAL_NVIC_EnableIRQ(TIM4_IRQn);
			switch(receivedSettings.relayId){
				case 1:
					targetHandle = &softwareTimerHandle_1;
					sprintf(task_name, "SoftTimer_1");
					timerParams.port = RLY1_GPIO_Port;
					timerParams.pin = RLY1_Pin;
					configureForSoftwareTimer(RLY1_GPIO_Port, RLY1_Pin);
					suspendRelayHardwareTimer(1);
					swTimersUsed |= 0x1;
					break;
				case 2:
					targetHandle = &softwareTimerHandle_2;
					sprintf(task_name, "SoftTimer_2");
					timerParams.port = RLY2_GPIO_Port;
					timerParams.pin = RLY2_Pin;
					configureForSoftwareTimer(RLY2_GPIO_Port, RLY2_Pin);
					suspendRelayHardwareTimer(2);
					swTimersUsed |= 0x2;
					break;
				case 3:
					targetHandle = &softwareTimerHandle_3;
					sprintf(task_name, "SoftTimer_3");
					timerParams.port = RLY3_GPIO_Port;
					timerParams.pin = RLY3_Pin;
					configureForSoftwareTimer(RLY3_GPIO_Port, RLY3_Pin);
					suspendRelayHardwareTimer(3);
					swTimersUsed |= 0x4;
					break;
			}
			if (*targetHandle != NULL) { // if task already exists, delete it and create new
				vTaskDelete(*targetHandle);
				*targetHandle = NULL;
			}

			BaseType_t result = xTaskCreate(software_timer_task, task_name, 256, &timerParams, 4, targetHandle);

		    if (result != pdPASS) {
		        // Handle error (e.g., log or retry)
		    	*targetHandle = NULL;
		        char tempBufferErr[30];
				sprintf(tempBufferErr, "SW Timer %lu task fault.", receivedSettings.relayId);
				printf("%s\n", tempBufferErr);
				reportErrorToPopUp(OTHER_ERR, tempBufferErr, 0);
		    }

		}else{ // USE HW TIMERS

			switch(receivedSettings.relayId){
				case 1:
					if(softwareTimerHandle_1 != NULL){ // destroy running SW timer
						vTaskDelete(softwareTimerHandle_1);
						softwareTimerHandle_1 = NULL;
						configureForHardwareTimer(RLY1_GPIO_Port, RLY1_Pin, GPIO_AF1_TIM1);
						resumeRelayHardwareTimer(1);
						swTimersUsed &= ~((uint8_t)0x1);
					}

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
					if(softwareTimerHandle_2 != NULL){
						vTaskDelete(softwareTimerHandle_2);
						softwareTimerHandle_2 = NULL;
						configureForHardwareTimer(RLY2_GPIO_Port, RLY2_Pin, GPIO_AF9_TIM12);
						resumeRelayHardwareTimer(2);
						swTimersUsed &= ~((uint8_t)0x2);
					}

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
					if(softwareTimerHandle_3 != NULL){
						vTaskDelete(softwareTimerHandle_3);
						softwareTimerHandle_3 = NULL;
						configureForHardwareTimer(RLY3_GPIO_Port, RLY3_Pin, GPIO_AF3_TIM8);
						resumeRelayHardwareTimer(3);
						swTimersUsed &= ~((uint8_t)0x3);
					}

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

			if(swTimersUsed == 0x0){//no active SW timers
				HAL_NVIC_DisableIRQ(TIM4_IRQn);
			}
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

//			if(grblConnectionStatus == CONNECTED){
//				grblInstructionsUploaded = false; // this is to prevent TIM3 UE interrupt from sending "~"
////				xEventGroupSetBits(ehEventsGRBL, EVENT_GRBL_BIT_UPLOAD_GCODE_TO_GRBL_AND_START);
//
//				qPackage_laserParams_t packageToSend;
//				packageToSend.eventMask = EVENT_GRBL_BIT_UPLOAD_GCODE_TO_GRBL_AND_START;
//
//				BaseType_t xStatus = xQueueSend(qhTouchGFXToGRBLControl, &packageToSend, 0); // should be able to put in the queue as it will be emptied immediately
//				if (xStatus != pdTRUE) {
//					//TODO: report a problem
//					while(1){
//						HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
//					};
//				}
//			}

			TIM3 -> EGR = TIM_EGR_UG_Msk;
			xEventGroupSetBits(ehEventsSoftwareTimer,EVENT_SW_TIMER_BIT_LAUNCH);
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
							&settingsToSend.encoderControl.circumference_01mm,
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
			status = saveResolutionRadiusToEEPROM(&receivedSettings.encoderControl.resolution, &receivedSettings.encoderControl.radius_01mm, &receivedSettings.encoderControl.circumference_01mm);
//			status = saveLaserConsoleActiveToEEPROM(&receivedSettings.laserParams.laser_console_en);
		}
		if(status == EEPROM_SUCCESS && receivedSettings.settingsMask & SETTINGS_CONSOLE_EN_Bit){
			status = saveLaserConsoleActiveToEEPROM(&receivedSettings.laserParams.laser_console_en);
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


// filter for velocity
#define N 10  // Window size
double velocity_samples[N] = {0};
uint8_t sample_index = 0;

double get_filtered_velocity(double new_value) {
    velocity_samples[sample_index] = new_value;
    sample_index = (sample_index + 1) % N;

    double sum = 0;
    for (uint8_t i = 0; i < N; i++) {
        sum += velocity_samples[i];
    }
    return sum / (double)N;
}


/*
 * 	Resume and suspend hardware timers (e.g. when SW timer is used)
 * */
void resumeRelayHardwareTimer(int8_t idx){
	switch(idx){
		case 1:
			TIM1->SMCR |= 0b110 << TIM_SMCR_SMS_Pos;	// Trigger mode
			break;
		case 2:
			TIM12->SMCR |= 0b110 << TIM_SMCR_SMS_Pos;	// Trigger mode
			break;
		case 3:
			TIM8->SMCR |= 0b110 << TIM_SMCR_SMS_Pos;	// Trigger mode
			break;
	}
}

void suspendRelayHardwareTimer(int8_t idx){
	switch(idx){
		case 1:
			TIM1->SMCR &= ~(0b111 << TIM_SMCR_SMS_Pos); // Slave mode disabled (0b000)
			break;
		case 2:
			TIM12->SMCR &= ~(0b111 << TIM_SMCR_SMS_Pos);// Slave mode disabled (0b000)
			break;
		case 3:
			TIM8->SMCR &= ~(0b111 << TIM_SMCR_SMS_Pos);	// Slave mode disabled (0b000)
			break;
	}
}

/*
 * 	Disable relay timers by disabling slave mode
 * */
void disableRelayTimers(void){
	if(softwareTimerHandle_1 != NULL) {
		vTaskSuspend(softwareTimerHandle_1);}else{
		TIM1->SMCR &= ~(0b111 << TIM_SMCR_SMS_Pos); // Slave mode disabled (0b000)};
	}

	if(softwareTimerHandle_2 != NULL) {
		vTaskSuspend(softwareTimerHandle_2);
	}else{
		TIM12->SMCR &= ~(0b111 << TIM_SMCR_SMS_Pos);// Slave mode disabled (0b000)
	}

	if(softwareTimerHandle_3 != NULL) {
		vTaskSuspend(softwareTimerHandle_3);
	}else{
		TIM8->SMCR &= ~(0b111 << TIM_SMCR_SMS_Pos);	// Slave mode disabled (0b000)
	}
}

/*
 * 	Enable relay timers by enabling trigger mode
 * */
void enableRelayTimers(void){
	if(softwareTimerHandle_1 != NULL) {
		vTaskResume(softwareTimerHandle_1);
	}else{
		TIM1->SMCR |= 0b110 << TIM_SMCR_SMS_Pos;	// Trigger mode
	}

	if(softwareTimerHandle_2 != NULL) {
		vTaskResume(softwareTimerHandle_2);
	}else{
		TIM12->SMCR |= 0b110 << TIM_SMCR_SMS_Pos;	// Trigger mode
	}

	if(softwareTimerHandle_3 != NULL) {
		vTaskResume(softwareTimerHandle_3);
	}else{
		TIM8->SMCR |= 0b110 << TIM_SMCR_SMS_Pos;	// Trigger mode
	}
}


void configureForHardwareTimer(GPIO_TypeDef* port, uint16_t pin, uint8_t alternateFunction) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // De-init in case previously set to software GPIO
    HAL_GPIO_DeInit(port, pin);

    // Configure as alternate function for timer
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;       // Alternate function push-pull
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = alternateFunction; // e.g., GPIO_AF1_TIM1

    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void configureForSoftwareTimer(GPIO_TypeDef* port, uint16_t pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // De-init in case it's currently used by a peripheral
    HAL_GPIO_DeInit(port, pin);

    // Configure as general-purpose output (push-pull)
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;   // Software control
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(port, &GPIO_InitStruct);
}


