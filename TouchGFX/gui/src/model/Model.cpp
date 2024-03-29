#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

#include "FreeRTOS.h"
#include "queue.h"
#include "event_groups.h"
#include "qPackages.h"
#include "string.h"




Model::Model() :
				modelListener(0), relay1duration(0), relay1delay(0), relay2duration(
								0), relay2delay(0), relay3duration(0), relay3delay(
								0), amount(0), setLengthActual(0), currLength(0), lastStatus(OP_OK),
								fetchSettings(true), message(""), p_message(message), relaysActive(0)

{

}

void Model::tick() {
	BaseType_t xStatus;
	//enum statusId_t statusId;


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

				break;
			default:
				break;
		}
	}
	/*	Fetch data from non-volatile memory*/
	extern EventGroupHandle_t ehEvents;
	extern QueueHandle_t qhSettingsToGUI;
	extern QueueHandle_t qhStatusReport;
	qPackage_settings_t receivedSettings;
	qPackage_statusReport_t settingsStatusReport;

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

			//  Apply new settings on hardware
			reportToRelaySetupTask(1);
			reportToRelaySetupTask(2);
			reportToRelaySetupTask(3);
			reportToEncoderControlTask();
			if(relaysActive == 0){
				xEventGroupSetBits(ehEvents,EVENT_BIT_RELAYS_DEACTIVATE);
			}
			touchgfx::Texts::setLanguage(languageIdx);
			// TODO: reportToBrightnessControl()

			//	Report to have widgets updated in Screen 1
			settingsStatusReport.statusId = SETTINGS_LOAD_SUCCESS;
			settingsStatusReport.data = 0;
		}else{
			//	Loading from memory did not return SUCCESS
			settingsStatusReport.statusId = SETTINGS_LOAD_ERR;
			settingsStatusReport.data = 0;
		}
		xQueueSend(qhStatusReport, &settingsStatusReport, 0);
		}
}



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



void Model::saveEncoderSettings(){
	extern QueueHandle_t qhGUItoWriteSettings;
	qPackage_settings_t settingsPackage;

	settingsPackage.encoderControl = qPackage_encoderControl_t {(uint8_t) cuttingActive, setLength, resolution, radius_01mm};
	settingsPackage.settingsMask = SETTINGS_RES_RAD_Bit | SETTINGS_LENGTH_Bit;


	BaseType_t xStatus = xQueueSend(qhGUItoWriteSettings, &settingsPackage, 0); // should be able to put in the queue as it will be emptied immediately
	if (xStatus != pdTRUE) {
		while (1);
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
		while (1);
	}

	//immediateCut = false;

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
			while (1);
		}
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
		while(1);
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
