
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
	qPackage_encoderControl_t receivedPackage;
	BaseType_t xStatus;

	for(;;){
		xStatus = xQueueReceive(qhGUItoEncoderControl, &receivedPackage, portMAX_DELAY); // will stay in block mode indefinetely
		if(xStatus == errQUEUE_EMPTY){
			//TODO: report a problem
			while(1);
		}
		if(receivedPackage.isActive){
			TIM3 -> CR1 |= TIM_CR1_CEN_Msk;
		}else{
			TIM3 -> CR1 &= ~TIM_CR1_CEN_Msk;
		}
		if(receivedPackage.immCut){
			TIM3 -> EGR = TIM_EGR_UG_Msk;
		}

		//TODO setup a timer to measure diff between int and fp calculation
		//uint64_t newArr_int = ((uint64_t)512*receivedPackage.resolution*receivedPackage.length_01mm)/(3216*receivedPackage.radius_01mm);

		uint32_t newArr = (uint32_t)(0.5+(float)receivedPackage.resolution*receivedPackage.length_01mm/(2*3.1415926*receivedPackage.radius_01mm));

		if(newArr > 0xFFFF ){
			TIM3 -> ARR = (uint32_t) 0xFFFF;
		}else{
			TIM3 -> ARR = (uint32_t) newArr;
		}


		xStatus = xQueueSend(qhEncoderControlToReport, &receivedPackage, pdMS_TO_TICKS(10)); // forward settings to reportTask
		if(xStatus == errQUEUE_FULL){
			//TODO: report a problem
			while(1);
		}

	}

}

void reportTask(void *pvParameters){
	qPackage_encoderControl_t receivedPackage;
	qPackage_report_t packageToSend;
	BaseType_t xStatus;
	uint32_t resolution = 2048; //read from EEPROM
	uint32_t radius_01mm = 0;//read from EEPROM

	for(;;){
		xStatus = xQueueReceive(qhEncoderControlToReport, &receivedPackage, 0);
		if(xStatus == pdPASS){
			resolution = receivedPackage.resolution;
			radius_01mm = receivedPackage.radius_01mm;
		}


		packageToSend.setLengthActual_01mm  = (uint32_t)((float)TIM3->ARR*2*3.141592*radius_01mm/resolution+0.5);
		packageToSend.currLength_01mm = (uint32_t)((float)TIM3->CNT*2*3.141592*radius_01mm/resolution+0.5);
		packageToSend.ammount = (uint32_t) ((uint16_t) HAL_GetTick()); // TODO: setup a timer to count ammount;

		xStatus = xQueueOverwrite(qhReportToTouchGFX, &packageToSend);
		if(xStatus == errQUEUE_FULL){
			//TODO: report a problem
			while(1);
		}

		vTaskDelay(pdMS_TO_TICKS(10));
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
