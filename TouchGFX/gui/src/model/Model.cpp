#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <qPackages.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "event_groups.h"

Model::Model() :
				modelListener(0), relay1duration(0), relay1delay(0), relay2duration(
								0), relay2delay(0), relay3duration(0), relay3delay(
								0), ammount(0), setLengthActual(0)

{

}

void Model::tick() {
	extern QueueHandle_t qhReportToTouchGFX;
	extern QueueHandle_t qhStatusReport;
	qPackage_report_t packageReport;
	qPackage_statusReport_t packageStatusReport;
	BaseType_t xStatus;
	//enum statusId_t statusId;

//	BaseType_t xStatus = xQueuePeek(qhReportToTouchGFX, &receivedPackage,0);
	xStatus = xQueueReceive(qhReportToTouchGFX, &packageReport, 0);
	if (xStatus == pdPASS) {
		currLength = packageReport.currLength_01mm;
		ammount = packageReport.ammount;
	}
	xStatus = xQueueReceive(qhStatusReport, &packageStatusReport, 0);
	if (xStatus == pdPASS) {
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
			case SET_LENGTH_OF:

				break;
		}

	}
}

void Model::setRelay1delay(uint32_t delay) {
	relay1delay = delay;
}
void Model::setRelay1duration(uint32_t duration) {
//TODO: limit to max duration+delay (42s per timer)
	relay1duration = duration;
	reportToRelaySetupTask(1);

}

void Model::setRelay2delay(uint32_t delay) {
	relay2delay = delay;
}
void Model::setRelay2duration(uint32_t duration) {
	relay2duration = duration;
	reportToRelaySetupTask(2);
}

void Model::setRelay3delay(uint32_t delay) {
	relay3delay = delay;
}
void Model::setRelay3duration(uint32_t duration) {
	relay3duration = duration;
	reportToRelaySetupTask(3);
}

void Model::resetAmmount(void) {
	ammount = 0;
	extern EventGroupHandle_t ehEvents;
	xEventGroupSetBits(ehEvents,EVENT_BIT_RST_AMMOUNT);
// TODO: send a message to reset counting timer
}
void Model::enableCutting(bool enable) {
	cuttingActive = enable;
	reportToEncoderControlTask();
}

void Model::immCut() {
	immediateCut = true;
	extern EventGroupHandle_t ehEvents;
	xEventGroupSetBits(ehEvents,EVENT_BIT_IMM_CUT);
	//reportToEncoderControlTask();
}

void Model::updateSetLength(uint32_t newLength) {
	setLength = newLength;
	reportToEncoderControlTask();
}

void Model::reportToEncoderControlTask() {
	extern QueueHandle_t qhGUItoEncoderControl;
	qPackage_encoderControl_t package = { (uint8_t) cuttingActive,
					//(uint8_t) immediateCut,
					setLength, // unit: 0.1 mm
					2048, // resolution
					500 // radius unit: 0.1 mm, 500*0.0001m = 0.05m
					};
	BaseType_t xStatus = xQueueSend(qhGUItoEncoderControl, &package, 0); // should be able to put in the queue as it will be emptied immediately
	if (xStatus == pdFAIL) {
		while (1)
			;
	}

	immediateCut = false;

}

void Model::reportToRelaySetupTask(uint32_t id) {
	extern QueueHandle_t qhTouchGFXToRelaySetup;
	qPackage_relaySetup_t packageToSend;

	packageToSend.relayId = id;
	switch (id) {
		case 1:
			packageToSend.duration_ms = relay1duration;
			packageToSend.delay_ms = relay1delay;
			break;
		case 2:
			packageToSend.duration_ms = relay2duration;
			packageToSend.delay_ms = relay2delay;
			break;
		case 3:
			packageToSend.duration_ms = relay3duration;
			packageToSend.delay_ms = relay3delay;
			break;
	}

	BaseType_t xStatus = xQueueSend(qhTouchGFXToRelaySetup, &packageToSend, 0); // should be able to put in the queue as it will be emptied immediately
	if (xStatus == pdFAIL) {
		//TODO: report a problem
		while (1)
			;
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
uint32_t Model::getAmmount() {
	return ammount;
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

