/*
 * GRBL_control.c
 *
 *  Created on: Feb 6, 2025
 *      Author: Asus
 */

#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <cstdarg>  // Include this for va_list, va_start, and vsnprintf
#include <cstdio>   // Include this for vsnprintf()
#include <cmath>   // For mathematical functions
//#include <math.h>   // For mathematical functions

#define PI 3.14159265

#include <stdint.h>
//#include <stdio.h>
//#include <string.h>


#include "FreeRTOS.h"
//#include "main.h"
#include "qPackages.h"
#include "queue.h"
#include "event_groups.h"
#include "GRBL_control.hpp"
//#include "stm32f7xx_hal_uart.h"

//#include <gui_generated/containers/popUpUartConsoleContainerBase.hpp>

#ifdef __cplusplus
extern "C" {
#endif


// https://github.com/grbl/grbl/wiki/Interfacing-with-Grbl
//https://github.com/gnea/grbl/wiki/Grbl-v1.1-Commands // new repo grbl 1.1

//#define GRBL_STANDARD_TIMEOUT 200
#define GRBL_STANDARD_TIMEOUT 10000

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

enum grblConn_t disconnectGrblController(){
	grblInstructionsUploaded = false;
	expectingResponse = false;
//	disableVelocityTracking();
	grblConnectionStatus = NOT_CONNECTED;

//    char tempBufferErr[30];
//    sprintf(tempBufferErr, "fault: GRBL disconnected!");
//    printf("%s\n", tempBufferErr);
//	reportErrorToPopUp(UART_TX_NOT_OKED, tempBufferErr, 0);

	return NOT_CONNECTED;
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
	BaseType_t xStatus = xQueueReceive(qhUARTtoGRBL, &receivedData, pdMS_TO_TICKS(GRBL_STANDARD_TIMEOUT)); // will stay in block mode 100 ms

	expectingResponse = false;

	if((xStatus == errQUEUE_EMPTY) ||  (strncmp(receivedData.data,"\r\nGrbl 1.1x ['$' for help]",10) != 0)){
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

//		disconnectGrblController();
		return disconnectGrblController();
	}

	TIM3->DIER |= TIM_DIER_UIE; // | TIM_DIER_CC1IE;

//	enableVelocityTracking(); // this is not really part of GRBL

	printf("GRBL connected: %s\n", receivedData.data);
	addToUARTconsoleBuffer(receivedData.data, receivedData.length, true);

	grblConnectionStatus = CONNECTED;




	return grblConnectionStatus;



}



int16_t setupGRBL(){

//	// Set the Z-axis max travel to 0 to avoid soft limit errors:
////	sprintf(lines[0],"$132=0");
//	lines.push_back(std::string("$132=0"));

	if(requestHomingGRBL() != 0){
		return -1;
	};

	std::vector<std::pair<std::string, std::string>> lines;
/*	G17 XY plane,
 *  G21 use mm
 *	G40 cancel diameter compensation,
 *	G49 cancel length offset,
 *	G54 coordinate system 1,
 *	G80 cancel motion, REMOVED
 *	G90 absolute positioning,
 *	G94 feed/minute mode)
 */
//	sprintf(lines[3],"G17 G20 G40 G49 G54 G80 G90 G94");


	addFormattedLine(lines,"G17 G21 G40 G49 G54 G90 G94\r");

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
    if (sendCommands(lines) != 0) {
		char temp[30];
		sprintf(temp,"GRBL respd but setup faild");
		reportErrorToPopUp(UART_TX_NOT_OKED, temp, 0);
//		grblConnectionStatus = NOT_CONNECTED;
        return -1;  // Error
    }
    return 0;
}


/**
 * Do homing cycle.
 */
int16_t requestHomingGRBL(){
	std::vector<std::pair<std::string, std::string>> lines;
	addFormattedLine(lines,"$H\r");

    if (sendCommands(lines, 45000) != 0) {
    	char temp[30];
		sprintf(temp,"GRBL respd but homing faild");
		reportErrorToPopUp(UART_TX_NOT_OKED, temp, 0);
		grblConnectionStatus = NOT_CONNECTED;
		return disconnectGrblController(); // if homing failed, then disconnect
    }
    return 0;
}


/*
 * Sends commands to move head to new location.
 *
 * */
int16_t moveGRBL(double x, double y){
	std::vector<std::pair<std::string, std::string>> lines;

	x = -x;
	y = -y;
	addFormattedLine(lines,"G53 G0 X%0.3f Y%0.3f\r", x, y);

    if (sendCommands(lines) != 0) {
        return -1;  // Error
    }
    return 0;
}

/*
 * Sends commands to update
 *
 * */
int16_t updateOriginGRBL(double x, double y){

	x = -x;
	y = -y;

	std::vector<std::pair<std::string, std::string>> lines;
	// set new zero point for coord. system 1 at G54
	// move to new origin point
//	addFormattedLine(lines,"G53 G0 X%0.3f Y%0.3f\r", x, y);

	// Ensure motion is completed
//	addFormattedLine(lines,"M400\r"); // not supported by grbl

	// Wait for 1 second
//	addFormattedLine(lines,"G4 P3");

	// save loc x y as new origin
	addFormattedLine(lines,"G10 L2 P1 X%0.3f Y%0.3f\r", x, y);
	// save current location as new origin
//	addFormattedLine(lines,"G10 L20 P1 X0 Y0\r");
	//	G10 L2 P1 X20 Y10

	// go to new after-homing position
//	addFormattedLine(lines,"G53 G0 X%0.3f Y%0.3f\r", x-20, y+20);
//	addFormattedLine(lines,"G54 G0 X%0.3f Y%0.3f\r", (double)-20.0, (double)+20);
	addFormattedLine(lines,"G54 G0 X%0.3f Y%0.3f\r", (double)0.0, (double)0.0);

	// Wait for 1 second
	addFormattedLine(lines,"G4 P2\r");
		// set new after homing position
	addFormattedLine(lines,"G28.1\r");

    if (sendCommands(lines) != 0) {
        return -1;  // Error
    }
    return 0;
}


void enableGRBLcontrol() {
    // 2. Enable Capture Compare Interrupt for TIM1 in the NVIC
    HAL_NVIC_SetPriority(TIM3_IRQn, 5, 0);  // Priority level (1, 0)
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void disableGRBLcontrol() {
    // 2. Disable Capture Compare Interrupt for TIM1 in the NVIC
    HAL_NVIC_DisableIRQ(TIM3_IRQn);
}

void delay_ms(uint32_t ms) {
	vTaskDelay(pdMS_TO_TICKS(ms));
//    uint32_t start = HAL_GetTick();  // Get current time in milliseconds
//    while ((HAL_GetTick() - start) < ms) {
//        // Wait until the specified time has passed
//    }
}

/* This function should be called from FreeRTOS thread
 * because it goes to sleep while waiting for response from GRBL controller.
 * It uses HAL_UARTEx_ReceiveToIdle_IT(huart, (uint8_t*)pUARTRxGlobalBuffer, UART_RX_GLOBAL_BUFFER_SIZE)
 * to receive data. It sets variable exepctingResponse = true, to intercept response on UART RX
 * before forwarding data to console.
 *
 * beta, alpha in degrees. velocity and feedrate in mm/min.
 * */
int16_t assembleAndSendCommandsToGRBL_impl(uint16_t alpha, uint16_t beta, uint16_t width, uint16_t feedrate, double velocity, uint16_t origin_x0, uint16_t origin_y0, uint8_t alphaCutActive){

	if(grblInstructionsUploaded == true){
		return 0;
	}

	struct coords_t point_origin, point_A, point_B, point_C, point_D, point_E;

	point_origin.x = -(double)origin_x0/10.0;
	point_origin.y = -(double)origin_y0/10.0;


	double w = (double)width/10.0;
	double f = (double)feedrate;
	double v = velocity;

	double dx2;
	double dx4;
	double fAdj2;
	double fAdj4;

	int16_t valid;
	valid = getDeltaX_beta(w, f, -v, (double)beta/10.0, -150.0, 150.0, &dx2, &fAdj2);
	if(valid != 0){
		return valid;
	}

	if(alphaCutActive){
		valid = getDeltaX_alpha(w, f, -v, (double)alpha/10.0, -150.0, 150.0, &dx4, &fAdj4);
		if(valid != 0){
			return valid;
		}
	}

//	double l3 = 3.0; //mm
	double l3 = 3.0; //mm

//#define MOVEMENT_FEEDRATE (double)500.0
//	l3 = l3/(1+v/MOVEMENT_FEEDRATE); // correction so that l3 is always the same irrespectable of velocity

	//	double l1 = dx2 + l3 + dx4;
//	double l1 = -dx2 + l3 + -dx4;

	//* OLD COORDINATES */
//	point_A.x = 0.0;
//	point_A.y = 0.0;
//
//	point_B.x = point_A.x + l1;
//	point_B.y = point_A.y;
//
//	point_C.x = point_B.x + dx2;
//	point_C.y = point_B.y - w;
//
//	point_D.x = point_C.x - l3;
//	point_D.y = point_C.y;
//
//	point_E.x = point_D.x + dx4;
//	point_E.y = point_D.y + w;

	/* NEW COORDINATES */
	point_A.x = 0.0;
	point_A.y = 0.0;

	point_B.x = point_A.x;
	point_B.y = point_A.y;

	point_C.x = point_B.x + dx2;
	point_C.y = point_B.y - w;

	point_D.x = point_C.x - l3;
	point_D.y = point_C.y;

	point_E.x = point_D.x + dx4;
	point_E.y = point_D.y + w;

	valid = checkIfPointReachable(point_origin, point_A, std::string("A"));
	if(valid != 0){
		return valid;
	}
	valid = checkIfPointReachable(point_origin, point_B, std::string("B"));
	if(valid != 0){
		return valid;
	}
	valid = checkIfPointReachable(point_origin, point_C, std::string("C"));
	if(valid != 0){
		return valid;
	}

	if(alphaCutActive){
		valid = checkIfPointReachable(point_origin, point_D, std::string("D"));
		if(valid != 0){
			return valid;
		}
		valid = checkIfPointReachable(point_origin, point_E, std::string("E"));
		if(valid != 0){
			return valid;
		}
	}

	printf("Point A: X%0.3f Y%0.3f\n", point_A.x, point_A.y);
	printf("Point B: X%0.3f Y%0.3f\n", point_B.x, point_B.y);
	printf("Point C: X%0.3f Y%0.3f\n", point_C.x, point_C.y);

	if(alphaCutActive){
		printf("Point D: X%0.3f Y%0.3f\n", point_D.x, point_D.y);
		printf("Point E: X%0.3f Y%0.3f\n", point_E.x, point_E.y);
	}

	std::vector<std::pair<std::string, std::string>> lines;

# define UART_TX_TIMEOUT (uint32_t) 100

#ifdef VARIABLE_SPINDLE_MODE // in this mode, laser is automatically disabled when not in G1

//	sendMessageToGRBL_blocking_FreeRTOS(const char* message, uint16_t length, uint32_t timeout);

	//  ; Activate coordinate system G54
//	addFormattedLineNoResponse(lines,"G54\r"); // should already be set from updateOrigin()

	//  ; Move to point A
	addFormattedLineNoResponse(lines,"G0 X%0.3f Y%0.3f\r", point_A.x, point_A.y);

	//  ; Move to point B at feedrate FR (adjust F value as needed)
	addFormattedLineNoResponse(lines,"G0 X%0.3f Y%0.3f\r", point_B.x, point_B.y);

	//; Turn on the laser (modify S-value for power control)
//	addFormattedLineNoResponse(lines,"M3 S1000\r");
//	addFormattedLineNoResponse(lines,"M4 S1000\r");

	//  ; Move to point C
	addFormattedLineNoResponse(lines,"M4 S1000\rG1 X%0.3f Y%0.3f F%0.1f\rM4 S0\r", point_C.x, point_C.y, (float)fAdj2);

	//  ; Turn off the laser
//	addFormattedLineNoResponse(lines,"M4 S0\r");

	//  ; Move to point D
	addFormattedLineNoResponse(lines,"G0 X%0.3f Y%0.3f\r", point_D.x, point_D.y);

	//  ; Turn on the laser
//	addFormattedLineNoResponse(lines,"M3 S1000\r");
//	addFormattedLineNoResponse(lines,"M4 S1000\r");

	//  ; Move to point E
	addFormattedLineNoResponse(lines,"M4 S1000\rG1 X%0.3f Y%0.3f F%0.1f\rM4 S0\r", point_E.x, point_E.y, (float)fAdj4);

	//  ; Turn off the laser
//	addFormattedLineNoResponse(lines,("M5\r"));

#else
	//  ; Activate coordinate system G54
	//	addFormattedLineNoResponse(lines,"G54\r"); // should already be set from updateOrigin()

		//  ; Move to point A and B

//		addFormattedLine(lines,"G0 X%0.3f Y%0.3f\rG0 X%0.3f Y%0.3f\r", point_A.x, point_A.y, point_B.x, point_B.y);
		// Move to point B
		addFormattedLine(lines,"G0 X%0.3f Y%0.3f\r",  point_B.x, point_B.y);


		// Turn on the laser; Move to point C; Turn off the laser
		addFormattedLine(lines,"M4 S1000\rG1 X%0.3f Y%0.3f F%0.1f\rM5\r", point_C.x, point_C.y, (float)fAdj2);


		//  ; Turn on the laser
		if(alphaCutActive){
			//  ; Move to point D
			addFormattedLine(lines,"G0 X%0.3f Y%0.3f\r", point_D.x, point_D.y);
			addFormattedLine(lines,"M4 S1000\r");
			//  ; Move to point E
			addFormattedLine(lines,"G1 X%0.3f Y%0.3f F%0.1f\r", point_E.x, point_E.y, (float)fAdj4);
			//  ; Turn off the laser
			addFormattedLine(lines,("M5\r"));
		}


#endif
	//  ; Go to G28
	addFormattedLine(lines,"G28\r");

    // Send commands
    if (sendCommands(lines) != 0) {
        return -1;  // Error
    }

//    Sent command: !
//    Sent command: G54
//    Sent command: G0 X0.000 Y0.000
//    Sent command: G0 X-17.427 Y0.000
//    Sent command: M3 S1000
//    Sent command: G0 X-14.891 Y-30.200
//    Sent command: M5
//    Sent command: G0 X-4.891 Y-30.200
//    Sent command: M3 S1000
//    Sent command: G1 X-0.000 Y0.000 F450.2
//    Sent command: M5
//    Sent command: G28
//    Sent command: ~

//	expectingResponse = false;

	grblInstructionsUploaded = true;

	// All messages sent and acknowledged.
	return 0;
}




int16_t sendContinueToGRBL(){
    // Only send start command if commands were successfully transfered.
    if(grblInstructionsUploaded){
		grblInstructionsUploaded = false;

		const char line_11 = '~';
		int16_t status;
		status = sendMessageToGRBL_blocking_FreeRTOS(&line_11, 1, 5);
		return status;
	}
    return -1;
}


void TIM3_IRQHandler(void) {
    // Check if the capture compare interrupt flag is set
    if (TIM3->SR & TIM_SR_CC1IF) {
        // Clear the interrupt flag by writing 0
        TIM3->SR &= ~TIM_SR_CC1IF;

//		qPackage_laserParams_t packageToSend;
//
//		packageToSend.eventMask = EVENT_GRBL_BIT_UPLOAD_GCODE_TO_GRBL;
//
//		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//		BaseType_t xStatus = xQueueSendFromISR(qhTouchGFXToGRBLControl, &packageToSend, &xHigherPriorityTaskWoken); // should be able to put in the queue as it will be emptied immediately
//	    // Perform a context switch if necessary
//	    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//		if (xStatus != pdTRUE) {
//			//TODO: report a problem
//			while(1){
//				HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
//			};
//		}

	}
    else if (TIM3->SR & TIM_SR_UIF) {
        // Clear the interrupt flag by writing 0
        TIM3->SR &= ~TIM_SR_UIF;

        qPackage_laserParams_t packageToSend;
		packageToSend.eventMask = EVENT_GRBL_BIT_UPLOAD_GCODE_TO_GRBL_AND_START;

		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		BaseType_t xStatus = xQueueSendFromISR(qhTouchGFXToGRBLControl, &packageToSend, &xHigherPriorityTaskWoken); // should be able to put in the queue as it will be emptied immediately
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

		if (xStatus != pdTRUE) {
			//TODO: report a problem
	        char tempBufferErr[30];
	        sprintf(tempBufferErr, "TIM3_UP_Int: err xQueueSend");
	        printf("%s\n", tempBufferErr);
			reportErrorToPopUp(OTHER_ERR, tempBufferErr, 0);
		}

//        sendContinueToGRBL();

	}
}

void checkForAlarm(const char* message){
	// check if error or alarm received
	int alarm_id = extractGrblAlarmCode(pUARTRxGlobalBuffer);

	if(alarm_id != -1){
		char tempBufferErr[30];
		switch(alarm_id){
			case 1:{
				sprintf(tempBufferErr, "GRBL: HARD LIMIT TRIGGERED!");
				printf("%s\n", tempBufferErr);
				reportErrorToPopUpFromISR(GRBL_ALARM, tempBufferErr, 0);

				qPackage_laserParams_t packageToSend;
				packageToSend.eventMask = EVENT_GRBL_BIT_HOMING_IS_CONFIRMED;

				BaseType_t xHigherPriorityTaskWoken = pdFALSE;
				xQueueSendFromISR(qhTouchGFXToGRBLControl, &packageToSend, &xHigherPriorityTaskWoken); // should be able to put in the queue as it will be emptied immediately
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
				break;
			}
			default:
				sprintf(tempBufferErr, "GRBL: %s",pUARTRxGlobalBuffer);
				break;
		}
		printf("%s\n", tempBufferErr);
		reportErrorToPopUpFromISR(GRBL_ALARM, tempBufferErr, alarm_id);
	}
}

void checkForError(const char* message){
	int error_id = extractGrblErrorCode(pUARTRxGlobalBuffer);

	if(error_id != -1){
		char tempBufferErr[30];
		switch(error_id){
			default:
				sprintf(tempBufferErr, "GRBL: %s", pUARTRxGlobalBuffer);
				break;
		}
		printf("%s\n", tempBufferErr);
		reportErrorToPopUpFromISR(GRBL_ERROR, tempBufferErr, error_id);
	}
}

int extractGrblAlarmCode(const char* message) {
    const char* prefix = "ALARM:";
    char* ptr = strstr(message, prefix);
    if (ptr != nullptr) {
        ptr += strlen(prefix);  // move pointer past "ALARM:"
        return atoi(ptr);       // convert following characters to integer
    }
    return -1;  // error code if "ALARM:" not found
}

int extractGrblErrorCode(const char* message) {
    const char* prefix = "error:";
    char* ptr = strstr(message, prefix);
    if (ptr != nullptr) {
        ptr += strlen(prefix);  // move pointer past "error:"
        return atoi(ptr);       // convert following characters to integer
    }
    return -1;  // error code if "error:" not found
}

/* This callback requires FreeRTOS to be running.
 *
 * */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{

	checkForAlarm(pUARTRxGlobalBuffer);
	checkForError(pUARTRxGlobalBuffer);

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

#ifdef __cplusplus
}
#endif

/*************************************
 *
 * C++ functions (accessible from this file only)
 *
 *************************************/

//void addFormattedLine(std::vector<std::string>& lines, const char* format, ...) {
//    char tempBuffer[100];  // Buffer for formatted text
//    va_list args;
//    va_start(args, format);
//    vsnprintf(tempBuffer, sizeof(tempBuffer), format, args);
//    va_end(args);
//
//    lines.push_back(std::string(tempBuffer));
//}


// Function for no response
void addFormattedLineNoResponse(std::vector<std::pair<std::string, std::string>>& lines,
                      const char* commandFormat, ...)
{
    char commandBuffer[100];  // Buffer for command text

    va_list args;
    va_start(args, commandFormat);
    vsnprintf(commandBuffer, sizeof(commandBuffer), commandFormat, args);
    va_end(args);

    // Push the command with an empty response
    lines.emplace_back("", std::string(commandBuffer));
}

// Default function to add a formatted command with default a response "ok"
void addFormattedLine(std::vector<std::pair<std::string, std::string>>& lines,
                      const char* commandFormat, ...)
{
    char commandBuffer[100];  // Buffer for command text

    va_list args;
    va_start(args, commandFormat);
    vsnprintf(commandBuffer, sizeof(commandBuffer), commandFormat, args);
    va_end(args);

    // Push the command with an empty response
    lines.emplace_back("ok", std::string(commandBuffer));
}

// Function to add a formatted command **with** an expected response
void addFormattedLineWithResponse(std::vector<std::pair<std::string, std::string>>& lines,
                                  const char* responseFormat,
                                  const char* commandFormat, ...)
{
    char commandBuffer[100];  // Buffer for command text
    char responseBuffer[100]; // Buffer for expected response

    va_list args;
    va_start(args, commandFormat);
    vsnprintf(commandBuffer, sizeof(commandBuffer), commandFormat, args);
    va_end(args);

    va_start(args, commandFormat);
    vsnprintf(responseBuffer, sizeof(responseBuffer), responseFormat, args);
    va_end(args);

    // Push the command and expected response as a pair
    lines.emplace_back(std::string(responseBuffer), std::string(commandBuffer));
}

void sendResetCmd(){
	char msg[1];
	*msg = (char)RESET_CMD;
	sendMessageToGRBL_blocking_FreeRTOS_shouldShow(msg, 1, 100, false);
}

int16_t checkIfPointReachable(struct coords_t point_origin, struct coords_t point, std::string pointName){

	double absolute_size_X = 450;
	double absolute_size_Y = 300;

	struct coords_t point_absolute;

	point_absolute.x = point.x - point_origin.x;
	point_absolute.y = point.y - point_origin.y;

	if((point_absolute.x > absolute_size_X) || (point_absolute.x < 0 )){
        char tempBufferErr[30];
        sprintf(tempBufferErr, "p_%s.x: OoR: %.1f", pointName.data(), point.x);
        printf("%s\n", tempBufferErr);
		reportErrorToPopUp(DX_CALC_ERROR, tempBufferErr, 0);
		return -1; //
	}
	if((point_absolute.y > absolute_size_Y) || (point_absolute.y < 0 )){
        char tempBufferErr[30];
        sprintf(tempBufferErr, "p_%s.y: OoR: %.1f", pointName.data(), point.y);
        printf("%s\n", tempBufferErr);
		reportErrorToPopUp(DY_CALC_ERROR, tempBufferErr, 0);
		return -1; //
	}
	return 0;
}

int16_t getDeltaX_beta(double w, double f, double v, double betaInitial, double deltaX_min, double deltaX_max, double* deltaX, double* fAdj) {

	printf("getDeltaX_beta: w: %.1f, f: %.1f, v: %.1f, betaInitial: %.1f, deltaX_min: %.1f, deltaX_max: %.1f\n",w,f,v,betaInitial,deltaX_min,deltaX_max);

	if((betaInitial < 30) || (betaInitial > 150)){
        char tempBufferErr[30];
        sprintf(tempBufferErr, "Inv. angle b: %3.1f (30<x<150)", betaInitial);
        printf("%s\n", tempBufferErr);
        reportErrorToPopUp(DX_CALC_ERROR, tempBufferErr, 0);
		return -1; //
	}

	double betaDeg = betaInitial - 90;
	double beta = PI*betaDeg/180.0;

	double sinB = sin(beta);
	double cosB = cos(beta);
	double tanB = tan(beta);

	*deltaX = w*tanB-(v/f)*(w/cosB);

	if((*deltaX < deltaX_min) || (*deltaX > deltaX_max)){
        char tempBufferErr[30];
        sprintf(tempBufferErr, "OoR: delta: %3.1f mm.", *deltaX);
        printf("%s\n", tempBufferErr);
		reportErrorToPopUp(DX_CALC_ERROR, tempBufferErr, 0);
		return -1; //
	}

	printf("getDeltaX_beta: deltaX: %.1f, fAdj: $.1f\n", *deltaX);
	*fAdj = sqrt(f*f-2*sinB*v*f+v*v);

	return 0;
}

int16_t getDeltaX_alpha(double w, double f, double v, double alphaInitial, double deltaX_min, double deltaX_max, double* deltaX, double* fAdj) {

	printf("getDeltaX_alpha: w: %.1f, f: %.1f, v: %.1f, alphaInitial: %.1f, deltaX_min: %.1f, deltaX_max: %.1f\n",w,f,v,alphaInitial,deltaX_min,deltaX_max);

	if((alphaInitial < 30) || (alphaInitial > 150)){
        char tempBufferErr[30];
        sprintf(tempBufferErr, "Inv. angle a: %3.1f (30<x<150)", alphaInitial);
		reportErrorToPopUp(DX_CALC_ERROR, tempBufferErr, 0);
		return -1; //
	}

	double alphaDeg = 90 - alphaInitial;
	double alpha = PI*alphaDeg/180.0;

	double sinB = sin(alpha);
	double cosB = cos(alpha);
	double tanB = tan(alpha);

	*deltaX = w*tanB-(v/f)*(w/cosB);

	if((*deltaX < deltaX_min) || (*deltaX > deltaX_max)){
        char tempBufferErr[30];
        sprintf(tempBufferErr, "OoR: delta: %3.1f mm.", *deltaX);
		reportErrorToPopUp(DX_CALC_ERROR, tempBufferErr, 0);
		return -1; //
	}

	*fAdj = sqrt(f*f-2*sinB*v*f+v*v);
	printf("getDeltaX_alpha: deltaX: %.1f, fAdj: $.1f\n", *deltaX);

	return 0;
}


/*
 * Sends commands <numLines> to GRBL device. Return 0 on success and -1 on error.
 * */
//int16_t sendCommands(char lines[][32], uint8_t numLines) {
//int16_t sendCommands(std::vector<std::string>& lines) {
int16_t sendCommands(std::vector<std::pair<std::string, std::string>>& lines) {
	return sendCommands(lines, GRBL_STANDARD_TIMEOUT);
}

/*
 * Sends commands <numLines> to GRBL device. Return 0 on success and -1 on error.
 * */
//int16_t sendCommands(std::vector<std::string>& lines, uint32_t response_timeout) {
int16_t sendCommands(std::vector<std::pair<std::string, std::string>>& lines, uint32_t response_timeout) {

//	for (uint8_t i = 0; i < numLines; i++) {
	for (uint8_t i = 0; i < lines.size(); i++) {

		if(grblConnectionStatus != CONNECTED){
            char tempBufferErr[30];
            sprintf(tempBufferErr, "SendGRBLcommand: not conn.");
            reportErrorToPopUp(OTHER_ERR, tempBufferErr, 0);
            disconnectGrblController();
            return -1;
		}

        int16_t status;
        if(std::strlen(lines[i].first.data()) > 0){ // is response expected?
        	expectingResponse = true;
        }

        // Send command
        status = sendMessageToGRBL_blocking_FreeRTOS(lines[i].second.data(), std::strlen(lines[i].second.data()), UART_TX_TIMEOUT);

        if (status != 0) {
            expectingResponse = false;
            char tempBufferErr[30];
            sprintf(tempBufferErr, "UART TX Err. HAL: %d!", status);
            reportErrorToPopUp(UART_TX_ERR, tempBufferErr, 0);
            disconnectGrblController();
            return -1;
        }

        if(std::strlen(lines[i].first.data()) > 0){
			// Wait for response from GRBL
			qPackage_UART receivedData;
			BaseType_t xStatus = xQueueReceive(qhUARTtoGRBL, &receivedData, pdMS_TO_TICKS(response_timeout));

			expectingResponse = false;

			// Check if "ok" was received
	//        if ((xStatus == errQUEUE_EMPTY) || (receivedData.length != 4) || (strncmp(receivedData.data, "\r\nok", 4) != 0)) {
			if ((xStatus == errQUEUE_EMPTY) || (receivedData.length < 2) || (strncmp(receivedData.data, lines[i].first.data(), 2) != 0)) {
				printf("For command: '%s' received response: '%s'.\n",lines[i].second.data(),receivedData.data);

				char tempBufferErr[30];

				if (xStatus == errQUEUE_EMPTY) {
					sprintf(tempBufferErr, "No response.");
					disconnectGrblController();
				} else {
					sprintf(tempBufferErr, "Resp: %.23s", receivedData.data);
					addToUARTconsoleBuffer(receivedData.data, receivedData.length, true);
				}

				reportErrorToPopUp(UART_TX_NOT_OKED, tempBufferErr, 0);
				return -1;
			}

			// Normal behavior; "ok" was received
			addToUARTconsoleBuffer(receivedData.data, receivedData.length, true);

        }else{
//#define SEND_DELAY 100
//        	delay_ms(SEND_DELAY);
//        	// no response expected
        }

    }

    return 0;  // Success
}

uint16_t sendMessageToGRBL_blocking_FreeRTOS(const char* message, uint16_t length, uint32_t timeout){
	return sendMessageToGRBL_blocking_FreeRTOS_shouldShow(message, length, timeout, true);
}

uint16_t sendMessageToGRBL_blocking_FreeRTOS_shouldShow(const char* message, uint16_t length, uint32_t timeout, bool showOnConsole){


	if(length > 100 - 3){
		return -1;
	}
	char tempMessage[100] = {0};

	strncpy(tempMessage, message, length);
//	tempMessage[length-1] = '\r';
//	tempMessage[length] = '\n';

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

	printf("Sent command: %s\n", message);
	return 0;

}

void addToUARTconsoleBuffer(const char* pData, uint16_t Size, bool isRX_nTX){

	if(!(Size < UART_RX_GLOBAL_BUFFER_SIZE-1)){ // if size is not smaller than capacity; there must be one more character for terminating character
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


