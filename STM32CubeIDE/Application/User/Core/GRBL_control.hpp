/*
 * GRBL_control.hpp
 *
 *  Created on: Feb 6, 2025
 *      Author: Asus
 */
//
#include <stdbool.h>
#include "main.h"

#ifndef APPLICATION_USER_CORE_GRBL_CONTROL_H_
#define APPLICATION_USER_CORE_GRBL_CONTROL_H_


#ifdef __cplusplus
#include <vector>
#include <string>
extern "C" {  // Ensure C linkage for C functions
#endif

void enableGRBLcontrol();
void disableGRBLcontrol();
void delay_ms(uint32_t ms);

int16_t setupGRBL();
int16_t moveGRBL(double x, double y);
int16_t updateOriginGRBL(double x, double y);
//int16_t sendToHomeGRBL(){
//	sendToGRBL(1.0, 1.0);
//};
int16_t requestHomingGRBL();
enum grblConn_t initiateGrblController();
enum grblConn_t disconnectGrblController();
int16_t assembleAndSendCommandsToGRBL_impl(uint16_t alpha, uint16_t beta, uint16_t width, uint16_t feedrate, double velocity, uint16_t origin_x0, uint16_t origin_y0, uint8_t alphaCutActive);
int16_t sendContinueToGRBL();
int extractGrblAlarmCode(const char* message);
int extractGrblErrorCode(const char* message);
void checkForAlarm(const char* message);
void checkForError(const char* message);

#ifdef __cplusplus
}  // End extern "C"

void sendResetCmd();
// C++ functions
void addFormattedLine(std::vector<std::pair<std::string, std::string>>& lines, const char* format, ...);
void addFormattedLineWithResponse(std::vector<std::pair<std::string, std::string>>& lines,
                                  const char* responseFormat,
                                  const char* commandFormat, ...);
void addFormattedLineNoResponse(std::vector<std::pair<std::string, std::string>>& lines,
                      	  	  	  const char* commandFormat, ...);
void addToUARTconsoleBuffer(const char* pData, uint16_t Size, bool isRX_nTX);
uint16_t sendMessageToGRBL_blocking_FreeRTOS(const char* message, uint16_t length, uint32_t timeout);

uint16_t sendMessageToGRBL_blocking_FreeRTOS_shouldShow(const char* message, uint16_t length, uint32_t timeout, bool showOnConsole);
//int16_t sendCommands(char lines[][32], uint8_t numLines);
int16_t sendCommands(std::vector<std::pair<std::string, std::string>>& lines, uint32_t reponse_timeout);
int16_t sendCommands(std::vector<std::pair<std::string, std::string>>& lines);

int16_t checkIfPointReachable(struct coords_t point_origin, struct coords_t point, std::string pointName);
int16_t getDeltaX_alpha(double w, double f, double v, double alphaInitial, double deltaX_min, double deltaX_max, double* deltaX, double* fAdj);
int16_t getDeltaX_beta(double w, double f, double v, double betaInitial, double deltaX_min, double deltaX_max, double* deltaX, double* fAdj);

#endif // __cplusplus



#endif /* APPLICATION_USER_CORE_GRBL_CONTROL_H_ */
