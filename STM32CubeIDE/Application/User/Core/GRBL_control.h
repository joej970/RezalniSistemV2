/*
 * GRBL_control.h
 *
 *  Created on: Feb 6, 2025
 *      Author: Asus
 */

#include <stdbool.h>
#include "main.h"

#ifndef APPLICATION_USER_CORE_GRBL_CONTROL_H_
#define APPLICATION_USER_CORE_GRBL_CONTROL_H_

enum grblConn_t initiateGrblController();


void addToUARTconsoleBuffer(char* pData, uint16_t Size, bool isRX_nTX);

//uint16_t sendMessageToGRBL_blocking_baremetal(char* message, uint16_t length, uint32_t timeout);
uint16_t sendMessageToGRBL_blocking_FreeRTOS(char* message, uint16_t length, uint32_t timeout);

uint16_t sendMessageToGRBL_blocking_FreeRTOS_shouldShow(char* message, uint16_t length, uint32_t timeout, bool showOnConsole);

int16_t assembleAndSendCommandsToGRBL();
int16_t assembleAndSendCommandsToGRBL_impl(uint16_t alpha, uint16_t beta, uint16_t width, uint16_t feedrate, double velocity, uint16_t origin_x0, uint16_t origin_y0);
int16_t sendContinueToGRBL();

void enableGRBLcontrol();
void disableGRBLcontrol();

int16_t sendCommands(char lines[][32], uint8_t numLines);
int16_t setupGRBL();
int16_t sendToOriginGRBL(double x, double y);
int16_t updateOriginGRBL(double x, double y);

#endif /* APPLICATION_USER_CORE_GRBL_CONTROL_H_ */
