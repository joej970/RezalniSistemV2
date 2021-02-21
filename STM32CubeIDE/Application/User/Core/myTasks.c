
/*
 * FreeRTOS tasks
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
 */
void encoderControlTask(void *pvParameters){
	qPacket_encoderControl_t receivedPacket;
	BaseType_t xStatus;

	for(;;){
		xStatus = xQueueReceive(qhGUItoEncoderControl, &receivedPacket, portMAX_DELAY);
	}


}
