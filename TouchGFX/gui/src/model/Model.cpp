#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

#include <iostream>

#include "FreeRTOS.h"
#include "queue.h"
#include "event_groups.h"
#include "string.h"

extern QueueHandle_t qhTouchGFXToGRBLControl;

void heartbeat(void)
{
	static uint16_t current_state = 0;


	// ‾‾‾.___.‾‾‾.___.___.___.___.___
	// 0   1   2   3   4   5   6   7
	// 32  64  96  128 160 192 224 256
	// 0   32  64  96  128 160 192 224
	//uint8_t stages[8] = {31,  63,  95,  127, 159, 191, 223, 255};
	//const uint16_t stages[8] = {31,  63,  95,  127, 159, 191, 223, 255};
	//uint8_t stages[8] = {0, 32,  64,  96,  128, 160, 192, 224};
	//const uint16_t start_beat_1 = 0;
	const uint16_t end_beat_1 = 31;
	const uint16_t start_beat_2 = 64;
	const uint16_t end_beat_2 = 95;
	const uint16_t cycle_period = 800;


	/*if(current_state <= stages[0]){
		HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
	}else if(current_state > stages[2] && current_state <= stages[3]){
		HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_RESET);
	}*/

	if(current_state <= end_beat_1){
		HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
	}else if(current_state > start_beat_2 && current_state <= end_beat_2){
		HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_RESET);
	}

	current_state += 8;

	if (current_state > cycle_period){
		current_state = 0;
	}




}

void toggleErrorLed(void)
{

    // Read the current state of the pin and toggle it
    if (HAL_GPIO_ReadPin(ERROR_LED_GPIO_Port, ERROR_LED_Pin) == GPIO_PIN_SET)
    {
        HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
    }
}


Model::Model() :
				modelListener(0), relay1duration(0), relay1delay(0), relay2duration(
								0), relay2delay(0), relay3duration(0), relay3delay(
								0), amount(0), setLengthActual(0), currLength(0), lastStatus(OP_OK),
								fetchSettings(true), message(""), p_message(message), relaysActive(0)

{
	clearConsoleBuffers();
}

void Model::clearConsoleBuffers(){

//	printf("Before clearconBufs:\n");
//	uint16_t i = 0;
//	for(auto lineBuffer : conBufs.lineBuffers){
//		printf("	line buffer[%d]: %s\n", i, lineBuffer.data());
//		i++;
//	}

	conBufs.resetConBuffers();

//	for(auto& lineBuffer : conBufs.lineBuffers){
//		lineBuffer.clear();
//	}
//
//	conBufs.currBuffer = conBufs.lineBuffers.size() - 1; // will be overflown to 0
//	conBufs.allLinesFull = false;
//	type_last = RX;

//	printf("After clearconBufs:\n");
//	i = 0;
//	for(auto lineBuffer : conBufs.lineBuffers){
//		printf("	line buffer[%d]: %s\n", i, lineBuffer.data());
//		i++;
//	}

}

void Model::processUARTData(qPackage_UART uartPayload){

	bool shouldBeOnTheSameLine = false;
	if((conBufs.type_last == TX) && (uartPayload.type == RX)){
		shouldBeOnTheSameLine = true;
	}

	conBufs.type_last = uartPayload.type;

	auto new_string = std::string(uartPayload.data, uartPayload.length);
	if(shouldBeOnTheSameLine){
		conBufs.decCurrBuffer();
		conBufs.lineBuffers[conBufs.getCurrBuffer()] += new_string;
	}
	else{
		conBufs.lineBuffers[conBufs.getCurrBuffer()] = new_string;
	}
//	conBufs.lineBuffers[conBufs.currBuffer] = std::string(uartPayload.data, uartPayload.length);

	conBufs.incCurrBuffer();

	//	conBufs.currBuffer++;
//	if(conBufs.currBuffer >= conBufs.lineBuffers.size()){
//		conBufs.currBuffer = 0;
//		conBufs.allLinesFull = true;
//	}

//	for(uint8_t i = 0; i < conBufs.lineBuffers.size(); i++){
//		if(i == conBufs.getCurrBuffer()){
//			printf("lineBuffer[%d]: %s <-- currentBuffer\n",i,conBufs.lineBuffers[i].data());
//		}else{
//			printf("lineBuffer[%d]: %s\n",i,conBufs.lineBuffers[i].data());
//		}
//	}

//	printf("idx: %d: strg message: %20s (%u)\n",
//			conBufs.currBuffer,
//			conBufs.lineBuffers[conBufs.currBuffer].data(),
//			conBufs.lineBuffers[conBufs.currBuffer].size());
//	printf("raw message: %20s (%u)\n",uartPayload.data, uartPayload.length);

	tellConsoleVisible();
//	uartShouldBeVisible = true;

}


void Model::tellConsoleVisible(){
	modelListener->onConsoleDataUpdated();
}


void Model::tryToConnectGRBL(){
	qPackage_laserParams_t packageToSend;
	packageToSend.eventMask = EVENT_GRBL_BIT_INITIATE_GRBL_CONTROLLER;

	BaseType_t xStatus = xQueueSend(qhTouchGFXToGRBLControl, &packageToSend, 0);
}

void Model::sendGRBLtoHome(){
	qPackage_laserParams_t packageToSend;
	packageToSend.eventMask = EVENT_GRBL_BIT_SEND_HOME;

	BaseType_t xStatus = xQueueSend(qhTouchGFXToGRBLControl, &packageToSend, 0);
}

void Model::sendGRBLtoOrigin(){
	qPackage_laserParams_t packageToSend;
	packageToSend.eventMask = EVENT_GRBL_BIT_SEND_TO_ORIGIN;

	BaseType_t xStatus = xQueueSend(qhTouchGFXToGRBLControl, &packageToSend, 0);
}


std::vector<const char *> Model::fetchUartLineBuffers(){

	std::vector<const char *> pointers{conBufs.lineBuffers.size(), NULL};

	if(!conBufs.allLinesFull){
		for(uint16_t idx = 0; idx < conBufs.lineBuffers.size(); idx++){
			pointers[idx] = conBufs.lineBuffers[idx].data();
		}

	}else{
		uint16_t lines = conBufs.lineBuffers.size();
		for(uint16_t l = 0; l < lines; l++){ // for lines 0-9
			uint16_t idx = (conBufs.getCurrBuffer() + l) % lines;

//			printf("to pointer[%d] put data[%d] \n", l, idx);
			pointers[l] = conBufs.lineBuffers[idx].data();

			/*
			 * pIdx | buffer
			 * if currBuffer == 0
			 * 0    0
			 * 1    1
			 *
			 * 9	9
			 *
			 * if currBuffer == 1
			 * 0	1
			 * 1	2
			 * 2	3
			 * 3	4
			 * ..
			 * 7	8
			 * 8	9
			 * 9	0
			 *
			 * if currBuffer == 2
			 * 0	2
			 * 1	3
			 * 2	4
			 * ..
			 * 6	8
			 * 7	9
			 * 8	0
			 * 9	1
			 *
			 *
			 * */

		}
	}

//	printf("pointers length = %d \n", pointers.size());
////	std::vector<uint16_t> lengths(6);
//	for(uint16_t i = 0; i < conBufs.lineBuffers.size(); i++){
////		lengths[i] = strlen(pointers[i]);
////		printf("fetchBuffers[%u]: message lineBuffer: %20s (%u)\n", i, conBufs.lineBuffers[i].data(), strlen(conBufs.lineBuffers[i].data()));
//		printf("pointer[%u]: message: %s (%u)\n", i, pointers[i], strlen(pointers[i]));
//	}

	return pointers;
}


void Model::tick() {
	BaseType_t xStatus;
	//enum statusId_t statusId;

	//toggleErrorLed();
	heartbeat();

	/* Check UART Console queue */
	extern QueueHandle_t qhUARTtoConsole;
	qPackage_UART uartPayload;
	xStatus = xQueueReceive(qhUARTtoConsole, &uartPayload, pdMS_TO_TICKS(0));
	if (xStatus == pdTRUE) {
		processUARTData(uartPayload);
	}

 	/* Receive status update */
	extern QueueHandle_t qhReportToTouchGFX;
	qPackage_report_t packageReport;
	xStatus = xQueueReceive(qhReportToTouchGFX, &packageReport, 0);
	if (xStatus == pdTRUE) {
		currLength = packageReport.currLength_01mm;
		amount = packageReport.amount;
	}
	/*	Receive statusReport from eeprom write, encoder & relay set up	*/
	extern QueueHandle_t qhStatusReport;
	qPackage_statusReport_t packageStatusReport;
	xStatus = xQueueReceive(qhStatusReport, &packageStatusReport, 0);
	if (xStatus == pdTRUE) {
		lastStatus = packageStatusReport.statusId; // this value is used in Screen1MainCuttingView.cpp to show notification
		statusPackageData = packageStatusReport.data;
		strcpy(message, packageStatusReport.message);
		printf("Received message: %s \n", message);
		switch (packageStatusReport.statusId) {
			case SET_LENGTH_TRIMMED:
				setLengthActual = packageStatusReport.data;
				break;
			case SET_LENGTH_VALID:
				setLengthActual = packageStatusReport.data;
				break;
			case RELAY_DELAY_OF:

				break;
			case RELAY_DURATION_OF:

				break;
			case RELAY_DEACTIVATED:

				break;
			case SETTINGS_LOAD_ERR:
				printf("Received SETTINGS_LOAD_ERR \n");
				break;
			case UART_TX_NOT_OKED:
				printf("Received UART_TX_NOT_OKED \n");
				break;
			default:
				printf("Received some other report message ID.\n");
				break;
		}
	}
	/*	Fetch data from non-volatile memory*/
	extern EventGroupHandle_t ehEvents;
	extern QueueHandle_t qhSettingsToGUI;
	extern QueueHandle_t qhStatusReport;
	qPackage_settings_t receivedSettings;
	qPackage_statusReport_t settingsStatusReport;

	//fetchSettings = false;
	if(fetchSettings){
		xEventGroupSetBits(ehEvents,EVENT_BIT_LOAD_SETTINGS);
		fetchSettings = false;
	}
	//	Now wait for the message. Transfer should take (address + memory address + 32 bytes) * (8 bits + ACK) = 34*9*1/400000 = 0.765 ms
	xStatus = xQueueReceive(qhSettingsToGUI, &receivedSettings, pdMS_TO_TICKS(0));
	if(xStatus == pdTRUE){
		if(receivedSettings.settingsMask & SETTINGS_ALL_Bit){
			resolution 		= receivedSettings.encoderControl.resolution;
			radius_01mm 	= receivedSettings.encoderControl.radius_01mm;
			setLength 		= receivedSettings.encoderControl.length_01mm;
			relay1duration  = receivedSettings.relay1.duration_ms;
			relay1delay  	= receivedSettings.relay1.delay_ms;
			relay2duration  = receivedSettings.relay2.duration_ms;
			relay2delay  	= receivedSettings.relay2.delay_ms;
			relay3duration  = receivedSettings.relay3.duration_ms;
			relay3delay  	= receivedSettings.relay3.delay_ms;
			languageIdx		= (LANGUAGES) receivedSettings.languageIdx;
			relaysActive	= receivedSettings.relaysActive;
			brightness		= receivedSettings.brightness;

			angle_alpha_01deg = receivedSettings.laserParams.angle_alpha_01deg;
			angle_beta_01deg = receivedSettings.laserParams.angle_beta_01deg;
			feedrate = receivedSettings.laserParams.feedrate;
			width_01mm = receivedSettings.laserParams.width_01mm;
			origin_y0_01mm = receivedSettings.laserParams.origin_y0_01mm;
			origin_x0_01mm = receivedSettings.laserParams.origin_x0_01mm;


			//  Apply new settings on hardware
			reportToRelaySetupTask(1);
			reportToRelaySetupTask(2);
			reportToRelaySetupTask(3);
			reportToEncoderControlTask();
			reportToGRBLControlTask();
			if(relaysActive == 0){
				xEventGroupSetBits(ehEvents,EVENT_BIT_RELAYS_DEACTIVATE);
			}
			touchgfx::Texts::setLanguage(languageIdx);
			// TODO: reportToBrightnessControl()

			//	Report to have widgets updated in Screen 1
			settingsStatusReport.statusId = SETTINGS_LOAD_SUCCESS;
			settingsStatusReport.data = 0;

		}else if(receivedSettings.settingsMask & SETTINGS_LASER_PARAMS_Bit){
			angle_alpha_01deg = receivedSettings.laserParams.angle_alpha_01deg;
			angle_beta_01deg = receivedSettings.laserParams.angle_beta_01deg;
			feedrate = receivedSettings.laserParams.feedrate;
			width_01mm = receivedSettings.laserParams.width_01mm;
			origin_y0_01mm = receivedSettings.laserParams.origin_y0_01mm;
			origin_x0_01mm = receivedSettings.laserParams.origin_x0_01mm;

			laserParamsUpdatedFromEEPROM = true;
		}

		else{
			//	Loading from memory did not return SUCCESS
			settingsStatusReport.statusId = SETTINGS_LOAD_ERR;
			settingsStatusReport.data = 0;
		}
		xQueueSend(qhStatusReport, &settingsStatusReport, 0);
	}
}


void Model::setAlpha(uint16_t angle) {
	angle_alpha_01deg = angle;
}

void Model::setBeta(uint16_t beta) {
	angle_beta_01deg = beta;
}

void Model::setFeedrate(uint16_t fr) {
	feedrate = fr;
}

void Model::setWidth(uint16_t width){
	width_01mm = width;
}


/* RELAY SETTERS */
void Model::setRelay1delay(uint32_t delay) {
	relay1delay = delay;
}
void Model::setRelay1duration(uint32_t duration) {
	relay1duration = duration;
	reportToRelaySetupTask(1);
	saveRelaySettings(1);

}

void Model::setRelay2delay(uint32_t delay) {
	relay2delay = delay;
}
void Model::setRelay2duration(uint32_t duration) {
	relay2duration = duration;
	reportToRelaySetupTask(2);
	saveRelaySettings(2);
}

void Model::setRelay3delay(uint32_t delay) {
	relay3delay = delay;
}
void Model::setRelay3duration(uint32_t duration) {
	relay3duration = duration;
	reportToRelaySetupTask(3);
	saveRelaySettings(3);
}

/* OTHER SETTERS */
void Model::setRadius(uint16_t radius){
	radius_01mm = radius;
	reportToEncoderControlTask();
	saveEncoderSettings();
}


void Model::resetLastStatus(){
	lastStatus = OP_OK;
}

void Model::resetAmount(void) {
	amount = 0;
	extern EventGroupHandle_t ehEvents;
	xEventGroupSetBits(ehEvents,EVENT_BIT_RST_AMOUNT);
}

void Model::resetCurrLength(void) {
	currLength = 0;
	extern EventGroupHandle_t ehEvents;
	xEventGroupSetBits(ehEvents,EVENT_BIT_RST_CURR_LEN);
}


void Model::enableCutting(bool enable) {
	cuttingActive = enable;
	reportToEncoderControlTask();
}

void Model::immCut() {
	extern EventGroupHandle_t ehEvents;
	xEventGroupSetBits(ehEvents,EVENT_BIT_IMM_CUT);
}

void Model::updateSetLength(uint32_t newLength) {
	setLength = newLength;
	reportToEncoderControlTask();
	saveEncoderSettings();
}

void Model::updateLaserParamsFromEEPROM(uint8_t slot) {
	extern EventGroupHandle_t ehEvents;

	switch(slot){
		case 1:
			xEventGroupSetBits(ehEvents,EVENT_BIT_LOAD_LASER_PARAMS_SLOT_1);
			break;
		case 2:
			xEventGroupSetBits(ehEvents,EVENT_BIT_LOAD_LASER_PARAMS_SLOT_2);
			break;
		case 3:
			xEventGroupSetBits(ehEvents,EVENT_BIT_LOAD_LASER_PARAMS_SLOT_3);
			break;
	}

}


/* SAVE TO EEPROM*/

void Model::saveEncoderSettings(){
	extern QueueHandle_t qhGUItoWriteSettings;
	qPackage_settings_t settingsPackage;

	settingsPackage.encoderControl = qPackage_encoderControl_t {(uint8_t) cuttingActive, setLength, resolution, radius_01mm};
	settingsPackage.settingsMask = SETTINGS_RES_RAD_Bit | SETTINGS_LENGTH_Bit;


	BaseType_t xStatus = xQueueSend(qhGUItoWriteSettings, &settingsPackage, 0); // should be able to put in the queue as it will be emptied immediately
	if (xStatus != pdTRUE) {
		while(1){
			HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
		};
	}
}

void Model::writeLaserParamsToEEPROM(uint8_t slot){
	extern QueueHandle_t qhGUItoWriteSettings;
	qPackage_settings_t settingsPackage;

	if(slot > 3){
		return;
	}

	settingsPackage.laserParams.origin_x0_01mm = origin_x0_01mm;
	settingsPackage.laserParams.origin_y0_01mm = origin_y0_01mm;
	settingsPackage.laserParams.angle_alpha_01deg = angle_alpha_01deg;
	settingsPackage.laserParams.angle_beta_01deg = angle_beta_01deg;
	settingsPackage.laserParams.feedrate = feedrate;
	settingsPackage.laserParams.width_01mm = width_01mm;
	settingsPackage.laserParams.slot = slot;

	settingsPackage.settingsMask = SETTINGS_LASER_PARAMS_Bit;

	BaseType_t xStatus = xQueueSend(qhGUItoWriteSettings, &settingsPackage, 0); // should be able to put in the queue as it will be emptied immediately
	if (xStatus == pdFAIL) {
		while(1){
			HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
		};
	}
}

void Model::saveRelaySettings(uint32_t id){
	extern QueueHandle_t qhGUItoWriteSettings;
	qPackage_settings_t settingsPackage;

	settingsPackage.settingsMask = SETTINGS_NONE_Bit;

		switch (id) {
			case 1:
				settingsPackage.relay1.duration_ms 	= relay1duration;
				settingsPackage.relay1.delay_ms 	= relay1delay;
				settingsPackage.settingsMask	 	= SETTINGS_RELAY1_Bit;
				break;
			case 2:
				settingsPackage.relay2.duration_ms 	= relay2duration;
				settingsPackage.relay2.delay_ms 	= relay2delay;
				settingsPackage.settingsMask	 	= SETTINGS_RELAY2_Bit;
				break;
			case 3:
				settingsPackage.relay3.duration_ms 	= relay3duration;
				settingsPackage.relay3.delay_ms 	= relay3delay;
				settingsPackage.settingsMask	 	= SETTINGS_RELAY3_Bit;
				break;
		}

		BaseType_t xStatus = xQueueSend(qhGUItoWriteSettings, &settingsPackage, 0); // should be able to put in the queue as it will be emptied immediately
		if (xStatus == pdFAIL) {
			while(1){
				HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
			};
		}
}

void Model::saveLanguage(LANGUAGES language){
	//	Save to memory
	extern QueueHandle_t qhGUItoWriteSettings;
	qPackage_settings_t settingsPackage;

	settingsPackage.settingsMask = SETTINGS_LANG_IDX_Bit;
	settingsPackage.languageIdx = (uint8_t) language;

	BaseType_t xStatus = xQueueSend(qhGUItoWriteSettings, &settingsPackage, 0); // should be able to put in the queue as it will be emptied immediately
	if (xStatus == pdFAIL) {
		while (1);
	}

}

/* REPORT TO TASKS */

void Model::reportToGRBLControlTask() {
	extern QueueHandle_t qhTouchGFXToGRBLControl;
	qPackage_laserParams_t packageToSend;

	packageToSend.angle_alpha_01deg = angle_alpha_01deg;
	packageToSend.angle_beta_01deg = angle_beta_01deg;
	packageToSend.feedrate = feedrate;
	packageToSend.width_01mm = width_01mm;
	packageToSend.origin_y0_01mm = origin_y0_01mm;
	packageToSend.origin_x0_01mm = origin_x0_01mm;
	packageToSend.eventMask = EVENT_GRBL_BIT_UPDATE_GRBL_PARAMS;

//	BaseType_t xStatus = xQueueOverwrite(qhTouchGFXToGRBLControl, &packageToSend); // should be able to put in the queue as it will be emptied immediately
	BaseType_t xStatus = xQueueSend(qhTouchGFXToGRBLControl, &packageToSend, 0); // should be able to put in the queue as it will be emptied immediately
	if (xStatus != pdTRUE) {
		//TODO: report a problem
		while(1){
			HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
		};
	}

}

void Model::reportToEncoderControlTask() {
	extern QueueHandle_t qhGUItoEncoderControl;

	BaseType_t xStatus;
	qPackage_encoderControl_t encoderPackage = {
					(uint8_t) cuttingActive,
					setLength, // unit: 0.1 mm
					resolution, // resolution
					radius_01mm // radius unit: 0.1 mm, 500*0.0001m = 0.05m
					};




	xStatus = xQueueSend(qhGUItoEncoderControl, &encoderPackage, 0); // should be able to put in the queue as it will be emptied immediately
	if (xStatus != pdTRUE) {
		while (1){
			HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
		};
	}

	//immediateCut = false;

}


void Model::reportToRelaySetupTask(uint32_t id) {
	extern QueueHandle_t qhTouchGFXToRelaySetup;
	qPackage_relaySetup_t packageToSend;

	packageToSend.relayId = id;
	switch (id) {
		case 1:
            packageToSend.duration_ms 	= relay1duration;
            packageToSend.delay_ms 		= relay1delay;
			break;
		case 2:
            packageToSend.duration_ms 	= relay2duration;
            packageToSend.delay_ms 		= relay2delay;
			break;
		case 3:
            packageToSend.duration_ms 	= relay3duration;
            packageToSend.delay_ms 		= relay3delay;
			break;
	}
	BaseType_t xStatus = xQueueSend(qhTouchGFXToRelaySetup, &packageToSend, 0); // should be able to put in the queue as it will be emptied immediately
	if (xStatus != pdTRUE) {
		//TODO: report a problem
		while(1){
			HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
		};
	}


}

void Model::toggleRelaysActive(){
	relaysActive = !relaysActive;
	extern EventGroupHandle_t ehEvents;
	if(relaysActive){
		xEventGroupSetBits(ehEvents,EVENT_BIT_RELAYS_ACTIVATE);
	}else{
		xEventGroupSetBits(ehEvents,EVENT_BIT_RELAYS_DEACTIVATE);
	}

	//	Save to memory
	extern QueueHandle_t qhGUItoWriteSettings;
	qPackage_settings_t settingsPackage;

	settingsPackage.settingsMask = SETTINGS_RELAY_ACT_Bit;
	settingsPackage.relaysActive = relaysActive;

	BaseType_t xStatus = xQueueSend(qhGUItoWriteSettings, &settingsPackage, 0); // should be able to put in the queue as it will be emptied immediately
	if (xStatus == pdFAIL) {
		while (1);
	}

}




/* GETTERS */


uint16_t Model::getAlpha(){return angle_alpha_01deg;}
uint16_t Model::getBeta(){return angle_beta_01deg;}
uint16_t Model::getFeedrate(){return feedrate;}
uint16_t Model::getWidth(){return width_01mm;}


uint32_t Model::getRelay1duration() {
	return relay1duration;
}
uint32_t Model::getRelay1delay() {
	return relay1delay;
}
uint32_t Model::getRelay2duration() {
	return relay2duration;
}
uint32_t Model::getRelay2delay() {
	return relay2delay;
}
uint32_t Model::getRelay3duration() {
	return relay3duration;
}
uint32_t Model::getRelay3delay() {
	return relay3delay;
}

uint16_t Model::getRadius(){
	return radius_01mm;
}
uint32_t Model::getAmount() {
	return amount;
}
bool Model::getCutting() {
	return cuttingActive;
}
uint32_t Model::getSetLength() {
	return setLength;
}
uint32_t Model::getSetLengthActual() {
	return setLengthActual;
}
uint32_t Model::getCurrLength() {
	return currLength;
}
statusId_t Model::getLastStatus(){
	return lastStatus;
}
uint32_t Model::getStatusPackageData(){
	return statusPackageData;
}
char* Model::getMessage(){
	return p_message;
}
uint8_t Model::getRelaysActive(){
	return relaysActive;
}
