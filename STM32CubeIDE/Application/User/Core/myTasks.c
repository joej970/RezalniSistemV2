
/*
 * FreeRTOS tasks
 *
 *  void ATaskFunction( void *pvParameters );
 *
 * 	BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,
 *		const char * const pcName,
 *		uint16_t usStackDepth,
 *		void *pvParameters,
 *		UBaseType_t uxPriority,
 *		TaskHandle_t *pxCreatedTask );
 *
 */
#include "myTasks.h"



/*
 * A task that waits to receive instructions about set length,
 * and whether the timer should be active and counting encoder pulses.
 * It generates update event when immediate cut instruction is received.
 *
 * This task should be higher priority than GUI task, so it pre-empts the GUI task when data is available.
 *
 * Package structure contains: uint8_t isActive; uint8_t immCut; uint32_t length;
} qPacket_encoderControl_t;
 */
void encoderControlTask(void *pvParameters){
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


		//TODO setup a timer to measure diff between int and fp calculation
		//uint64_t newArr_int = ((uint64_t)512*receivedSettings.resolution*receivedSettings.length_01mm)/(3216*receivedSettings.radius_01mm);

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

void reportTask(void *pvParameters){
	qPackage_encoderControl_t receivedSettings;
	qPackage_report_t packageToSend;
	BaseType_t xStatus;
	uint32_t resolution = 2048; //read from EEPROM
	uint32_t radius_01mm = 0;//read from EEPROM

	for(;;){
		xStatus = xQueueReceive(qhEncoderControlToReport, &receivedSettings, 0);
		if(xStatus == pdPASS){
			resolution = receivedSettings.resolution;
			radius_01mm = receivedSettings.radius_01mm;
		}


		packageToSend.currLength_01mm = (uint32_t)((float)TIM3->CNT*2*3.141592*radius_01mm/resolution+0.5);
		packageToSend.ammount = TIM5 -> CNT;

		xStatus = xQueueOverwrite(qhReportToTouchGFX, &packageToSend);
		if(xStatus == errQUEUE_FULL){
			//TODO: report a problem, this always return true anyway
			while(1);
		}



		vTaskDelay(pdMS_TO_TICKS(10));
	}

}

void relaySetupTask(void *pvParameters){
	qPackage_relaySetup_t receivedSettings;
	qPackage_statusReport_t packageToSend;
	BaseType_t xStatus;
	enum statusId_t statusId;

	for(;;){
		xStatus = xQueueReceive(qhTouchGFXToRelaySetup, &receivedSettings, portMAX_DELAY); // will stay in block mode indefinetely
		if(xStatus == errQUEUE_EMPTY){
			//TODO: report a problem
			while(1);
		}
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
				statusId = RELAY_ACTIVATED;
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
		eventBits = xEventGroupWaitBits(ehEvents, EVENT_BITS_ALL, pdTRUE, pdFALSE, portMAX_DELAY);

		if(eventBits & EVENT_BIT_RST_AMMOUNT){
			TIM5 -> CNT = 0;
		}
		if(eventBits & EVENT_BIT_IMM_CUT){
			TIM3 -> EGR = TIM_EGR_UG_Msk;
			TIM5 -> CNT -=1; // cutting a new piece increments ammount counter
		}
	}


}


//void StartTouchGFXTask(void * argument)
//{
//  /* USER CODE BEGIN 5 */
//  MX_TouchGFX_Process();
//  /* Infinite loop */
//  for(;;)
//  {
//	  vTaskDelay(pdMS_TO_TICKS(1));
//  }
//  /* USER CODE END 5 */
//}
