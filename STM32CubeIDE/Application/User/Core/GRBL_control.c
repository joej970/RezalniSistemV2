/*
 * GRBL_control.c
 *
 *  Created on: Feb 6, 2025
 *      Author: Asus
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>


#include "FreeRTOS.h"
//#include "main.h"
#include "qPackages.h"
#include "queue.h"
#include "event_groups.h"
//#include "stm32f7xx_hal_uart.h"

//#include <gui_generated/containers/popUpUartConsoleContainerBase.hpp>
#include "GRBL_control.h"

// https://github.com/grbl/grbl/wiki/Interfacing-with-Grbl
//https://github.com/gnea/grbl/wiki/Grbl-v1.1-Commands // new repo grbl 1.1

//#define GRBL_TIMEOUT 200
#define GRBL_TIMEOUT 10000

bool expectingResponse = false;
bool grblInstructionsUploaded = false;
bool grblIsBusy = false;

struct coords_t {
	double x;
	double y;
};

extern char* pUARTRxGlobalBuffer;
extern QueueHandle_t qhUARTtoConsole;
extern QueueHandle_t qhUARTtoGRBL;
extern QueueHandle_t qhStatusReport;
//extern EventGroupHandle_t ehEventsGRBL;
extern UART_HandleTypeDef huart7;
extern enum grblConn_t grblConnectionStatus;
extern QueueHandle_t qhTouchGFXToGRBLControl;

#define RESET_CMD 0x18


void sendResetCmd(){
	char msg[1];
	*msg = (char)RESET_CMD;
	sendMessageToGRBL_blocking_FreeRTOS_shouldShow(msg, 1, 100, false);
}

/* Checks for GRBL device on UART7. Returns state of connection.
 * Needs to be called from FreeRTOS task.
 *
 * */
enum grblConn_t initiateGrblController(){

	grblConnectionStatus = PENDING;

	expectingResponse = true;
	// send reset
	printf("Searching for GRBL controller...\n");
	sendResetCmd();

	// Queue to receive response from the GRBL controller



	qPackage_UART receivedData;
	BaseType_t xStatus = xQueueReceive(qhUARTtoGRBL, &receivedData, pdMS_TO_TICKS(GRBL_TIMEOUT)); // will stay in block mode 100 ms

	expectingResponse = false;

	if((xStatus == errQUEUE_EMPTY) ||  (strncmp(receivedData.data,"Grbl vX.Xx ['$' for help]",4) != 0)){
		qPackage_statusReport_t packageStatusReport;
		packageStatusReport.statusId = UART_TX_NOT_OKED;
		packageStatusReport.data = 0;

		if(xStatus == errQUEUE_EMPTY){
			// Report status to popUpWindow
			sprintf(packageStatusReport.message, "No resp. from GRBL.");
			printf("No response from GRBL. \n");
		}else{
			// Report status to popUpWindow
			snprintf(packageStatusReport.message, sizeof(packageStatusReport.message), "Resp: %.23s", receivedData.data);
			// Send response to console viewer.
			printf("Invalid response on UART: %s \n", receivedData.data);
			addToUARTconsoleBuffer(receivedData.data, receivedData.length, true);
		}

		BaseType_t xStatus = xQueueSend(qhStatusReport, &packageStatusReport, 0);

		if(xStatus == errQUEUE_FULL){
			//TODO: report a problem
			while(1){
				HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
			};
		}

		grblConnectionStatus = NOT_CONNECTED;
		return NOT_CONNECTED;
	}

	TIM3->DIER |= TIM_DIER_UIE | TIM_DIER_CC1IE;

	printf("GRBL connected: %s\n", receivedData.data);
	addToUARTconsoleBuffer(receivedData.data, receivedData.length, true);

	setupGRBL();

	grblConnectionStatus = CONNECTED;
	return CONNECTED;

}



int16_t setupGRBL(){
	const uint8_t LINES_COUNT = 10;
	char lines [LINES_COUNT][32];

	// Set the Z-axis max travel to 0 to avoid soft limit errors:
	sprintf(lines[0],"$132=0");

	// Enable laser mode.
	sprintf(lines[1],"$32=1");

	// Execute homing
	sprintf(lines[2],"$H");

/*	G17 XY plane,
 *	G20 mm mode,
 *	G40 cancel diameter compensation,
 *	G49 cancel length offset,
 *	G54 coordinate system 1,
 *	G80 cancel motion,
 *	G90 absolute positioning,
 *	G94 feed/minute mode)
 */
	sprintf(lines[3],"G17 G20 G40 G49 G54 G80 G90 G94");


	/*
	TIP: After running a homing cycle, rather than jogging manually all the time to a position
	in the middle of your workspace volume. You can set a G28 or G30 pre-defined position
	to be your post-homing position, closer to where you'll be machining. To set these,
	you'll first need to jog your machine to where you would want it to move to after homing.
	Type G28.1 (or G30.1) to have Grbl store that position. So then after '$H' homing, you could
	just enter 'G28' (or 'G30') and it'll move there auto-magically.
	In general, I would just move the XY axis to the center and leave the Z-axis up.
	This ensures that there isn't a chance the tool in the spindle will interfere and
	that it doesn't catch on anything
	*/
//	sprintf(lines[2],"G28.1");


    // Send commands
    if (sendCommands(lines, 4) != 0) {
        return -1;  // Error
    }
}


/**
 * Do homing cycle.
 */
int16_t sendToHomeGRBL(){
	//  ; Stop execution (feed hold)
	const uint8_t LINES_COUNT = 1;
	char lines [LINES_COUNT][32];

	memset(lines, 0, sizeof(lines));

	sprintf(lines[0],"$H");

    // Send commands
    if (sendCommands(lines, 1) != 0) {
        return -1;  // Error
    }

    return 0;

}


/*
 * Sends commands to send head to new origin.
 *
 * */
int16_t sendToOriginGRBL(double x, double y){
//	//  ; Stop execution (feed hold)
	const uint8_t LINES_COUNT = 3;
	char lines [LINES_COUNT][32];
	memset(lines, 0, sizeof(lines));

	sprintf(lines[0],"G53 G0 X%0.3f Y%0.3f", x, y);

    // Send commands
    if (sendCommands(lines, 1) != 0) {
        return -1;  // Error
    }

    return 0;

}

/*
 * Sends commands to update
 *
 * */
int16_t updateOriginGRBL(double x, double y){
	const uint8_t LINES_COUNT = 3;
	char lines [LINES_COUNT][32];
	memset(lines, 0, sizeof(lines));

	sprintf(lines[0],"G53 G0 X%0.3f Y%0.3f", x, y);
	// update current position as X0 Y0 of G54 coord. system
	sprintf(lines[1],"G10 L20 P1 X0 Y0");

    // Send commands
    if (sendCommands(lines, 2) != 0) {
        return -1;  // Error
    }

    return 0;

}


void enableGRBLcontrol() {
    // 2. Enable Capture Compare Interrupt for TIM1 in the NVIC
    HAL_NVIC_SetPriority(TIM3_IRQn, 4, 0);  // Priority level (1, 0)
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void disableGRBLcontrol() {
    // 2. Disable Capture Compare Interrupt for TIM1 in the NVIC
    HAL_NVIC_DisableIRQ(TIM3_IRQn);
}




uint16_t sendMessageToGRBL_blocking_FreeRTOS(char* message, uint16_t length, uint32_t timeout){
	return sendMessageToGRBL_blocking_FreeRTOS_shouldShow(message, length, timeout, true);
}

uint16_t sendMessageToGRBL_blocking_FreeRTOS_shouldShow(char* message, uint16_t length, uint32_t timeout, bool showOnConsole){


	if(length > 100 - 2){
		return -1;
	}
	char tempMessage[100] = {0};

	strncpy(tempMessage, message, length);
	tempMessage[length] = '\r';

	HAL_StatusTypeDef status = HAL_UART_Transmit(&huart7, (uint8_t*)tempMessage, length + 1, timeout);

    // if not successful put to sleep and try again
	if(status == HAL_BUSY){
		vTaskDelay(pdMS_TO_TICKS(100));
		status = HAL_UART_Transmit(&huart7, (uint8_t*)tempMessage, length + 1, timeout);
	}

	if(status != HAL_OK){
		return (uint16_t)status;
	}else if(showOnConsole){
		addToUARTconsoleBuffer(tempMessage, length + 1, false);
	}
	return 0;

}






/* This function should be called from FreeRTOS thread
 * because it goes to sleep while waiting for response from GRBL controller.
 * It uses HAL_UARTEx_ReceiveToIdle_IT(huart, (uint8_t*)pUARTRxGlobalBuffer, UART_RX_GLOBAL_BUFFER_SIZE)
 * to receive data. It sets variable exepctingResponse = true, to intercept response on UART RX
 * before forwarding data to console.
 *
 * */
int16_t assembleAndSendCommandsToGRBL_impl(uint16_t alpha, uint16_t beta, uint16_t width, uint16_t feedrate, double velocity, uint16_t origin_x0, uint16_t origin_y0){

	if(grblInstructionsUploaded == true){
		return 0;
	}

	struct coords_t point_0, point_A, point_B, point_C, point_D, point_E;

	point_0.x = (double)origin_x0/10;
	point_0.y = (double)origin_y0/10;

	const uint8_t LINES_COUNT = 11;
	char lines [LINES_COUNT][32];
	memset(lines, 0, sizeof(lines));


# define UART_TX_TIMEOUT (uint32_t) 100
	//  ; Stop execution (feed hold)
	sprintf(lines[0],"!");

	//  ; Set new origin at current position
	sprintf(lines[1],"G92 X%0.2f Y%0.2f", point_0.x, point_0.y);

	//  ; Move to point A
	sprintf(lines[2],"G0 X%0.2f Y%0.2f", point_A.x, point_A.y);

	//  ; Move to point B at feedrate FR (adjust F value as needed)
	sprintf(lines[3],"G1 X%0.2f Y%0.2f F%0.1f", point_B.x, point_B.y, (float)feedrate);

	//; Turn on the laser (modify S-value for power control)
	sprintf(lines[4],"M3 S1000");

	//  ; Move to point C
	sprintf(lines[5],"G1 X%0.2f Y%0.2f", point_C.x, point_C.y);

	//  ; Turn off the laser
	sprintf(lines[6],"M5");

	//  ; Move to point D
	sprintf(lines[7],"G0 X%0.2f Y%0.2f", point_D.x, point_D.y);

	//  ; Turn on the laser
	sprintf(lines[8],"M3 S1000");

	//  ; Move to point E
	sprintf(lines[9],"G1 X%0.2f Y%0.2f F%0.1f", point_E.x, point_E.y, (float)feedrate);

	//  ; Turn off the laser
	sprintf(lines[10],"M5");

//	sprintf(lines[11],"~");//  ; Resume execution""


	/* Transfer the commands.*/
//	for(int i = 0; i < LINES_COUNT; i++){
//		int16_t status;
//
//		// Wait for response
//		expectingResponse = true;
//		status = sendMessageToGRBL_blocking_FreeRTOS(lines[i], strlen(lines[i]), UART_TX_TIMEOUT);
//
//		// Check if TX was successful
//		if(status != 0){
//			expectingResponse = false;
//
//			char tempBuffer[30];
//			sprintf(tempBuffer, "UART TX Err. HAL: %u!", status);
//			reportErrorToPopUp(UART_TX_ERR, tempBuffer, 0);
//
//		}
//
//		// Queue to receive response from the GRBL controller
//		qPackage_UART receivedData;
//		BaseType_t xStatus = xQueueReceive(qhUARTtoGRBL, &receivedData, pdMS_TO_TICKS(GRBL_TIMEOUT)); // will stay in block mode indefinetely
//
//		expectingResponse = false;
//
//		if((xStatus == errQUEUE_EMPTY) || (receivedData.length != 2) || (strncmp(receivedData.data,"ok",2) != 0)){
//
//			char tempBuffer[30];
//
//			qPackage_statusReport_t packageStatusReport;
//
//			if(xStatus == errQUEUE_EMPTY){
//				sprintf(tempBuffer, "No response.");
//			}else{
//				sprintf(tempBuffer, "Resp: %.23s", receivedData.data);
//				addToUARTconsoleBuffer(receivedData.data, receivedData.length, true);
//			}
//
//			reportErrorToPopUp(UART_TX_NOT_OKED, tempBuffer, 0);
//			// "ok" not received
//
//			grblConnectionStatus = NOT_CONNECTED;
//			return -1;
//		}
//
//		// normal behavior; "ok" was received
//		addToUARTconsoleBuffer(receivedData.data, receivedData.length, true);
//
//	}

    // Send commands
    if (sendCommands(lines, LINES_COUNT) != 0) {
        return -1;  // Error
    }

//	expectingResponse = false;

	grblInstructionsUploaded = true;

	// All messages sent and acknowledged.
	return 0;
}


/*
 * Sends commands <numLines> to GRBL device. Return 0 on success and -1 on error.
 * */
int16_t sendCommands(char lines[][32], uint8_t numLines) {
    for (uint8_t i = 0; i < numLines; i++) {
        int16_t status;
        expectingResponse = true;

        // Send command
        status = sendMessageToGRBL_blocking_FreeRTOS(lines[i], strlen(lines[i]), UART_TX_TIMEOUT);

        if (status != 0) {
            expectingResponse = false;
            char tempBuffer[30];
            sprintf(tempBuffer, "UART TX Err. HAL: %d!", status);
            reportErrorToPopUp(UART_TX_ERR, tempBuffer, 0);
            return -1;
        }

        // Wait for response from GRBL
        qPackage_UART receivedData;
        BaseType_t xStatus = xQueueReceive(qhUARTtoGRBL, &receivedData, pdMS_TO_TICKS(GRBL_TIMEOUT));

        expectingResponse = false;

        // Check if "ok" was received
        if ((xStatus == errQUEUE_EMPTY) || (receivedData.length != 2) || (strncmp(receivedData.data, "ok", 2) != 0)) {
            char tempBuffer[30];

            if (xStatus == errQUEUE_EMPTY) {
                sprintf(tempBuffer, "No response.");
            } else {
                sprintf(tempBuffer, "Resp: %.23s", receivedData.data);
                addToUARTconsoleBuffer(receivedData.data, receivedData.length, true);
            }

            reportErrorToPopUp(UART_TX_NOT_OKED, tempBuffer, 0);
            grblConnectionStatus = NOT_CONNECTED;
            return -1;
        }

        // Normal behavior; "ok" was received
        addToUARTconsoleBuffer(receivedData.data, receivedData.length, true);
    }

    return 0;  // Success
}

void TIM3_IRQHandler(void) {
    // Check if the capture compare interrupt flag is set
    if (TIM3->SR & TIM_SR_CC1IF) {
        // Clear the interrupt flag by writing 0
        TIM3->SR &= ~TIM_SR_CC1IF;

//		xEventGroupSetBitsFromISR(ehEventsGRBL, EVENT_GRBL_BIT_UPLOAD_GCODE_TO_GRBL);


		qPackage_laserParams_t packageToSend;

		packageToSend.eventMask = EVENT_GRBL_BIT_UPLOAD_GCODE_TO_GRBL;

		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		BaseType_t xStatus = xQueueSendFromISR(qhTouchGFXToGRBLControl, &packageToSend, &xHigherPriorityTaskWoken); // should be able to put in the queue as it will be emptied immediately
	    // Perform a context switch if necessary
	    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		if (xStatus != pdTRUE) {
			//TODO: report a problem
			while(1){
				HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
			};
		}


	}
    else if (TIM3->SR & TIM_SR_UIF) {
        // Clear the interrupt flag by writing 0
        TIM3->SR &= ~TIM_SR_UIF;
        sendContinueToGRBL();

	}
}

int16_t sendContinueToGRBL(){
    // Only send start command if commands were successfully transfered.
    if(grblInstructionsUploaded){
		grblInstructionsUploaded = false;
		char line_11[1] = "~";
		int16_t status;
		status = sendMessageToGRBL_blocking_FreeRTOS(line_11, 1, 5);
		return status;
	}
    return -1;
}


void addToUARTconsoleBuffer(char* pData, uint16_t Size, bool isRX_nTX){

	if(!(Size < UART_RX_GLOBAL_BUFFER_SIZE)){ // if size is not smaller than capacity; there must be one more character for terminating character
		return;
	}

	qPackage_UART uartPayload;

	strncpy(uartPayload.data, pData, Size);
	uartPayload.length = Size;
	uartPayload.data[Size] = '\0';
	if(isRX_nTX){
		uartPayload.type = RX;
	}else{
		uartPayload.type = TX;
	}

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	BaseType_t xStatus = xQueueSendFromISR(qhUARTtoConsole, &uartPayload, &xHigherPriorityTaskWoken);
    // Perform a context switch if necessary
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	if(xStatus != pdTRUE){
		//TODO: report a problem
		while(1){
			HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
		};
	}
}


/* This callback requires FreeRTOS to be running.
 *
 * */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{

	if(expectingResponse){ // if expecting response, intercept the message

		qPackage_UART uartPayload;

		strncpy(uartPayload.data, pUARTRxGlobalBuffer, Size);
		uartPayload.data[Size] = '\0';
		uartPayload.length = Size;
		uartPayload.type = RX;

		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		BaseType_t xStatus = xQueueSendFromISR(qhUARTtoGRBL, &uartPayload, &xHigherPriorityTaskWoken);
        // Perform a context switch if necessary
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

		if(xStatus != pdTRUE){
			//TODO: report a problem
			while(1){
				HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
			};
		}
	}else{
		addToUARTconsoleBuffer(pUARTRxGlobalBuffer, Size, true);
	}


	HAL_UARTEx_ReceiveToIdle_IT(huart, (uint8_t*)pUARTRxGlobalBuffer, UART_RX_GLOBAL_BUFFER_SIZE);
//	if(automaticReenable){
//	}

}
