#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <qPackages.h>
#include "FreeRTOS.h"
#include "queue.h"

Model::Model() :
	modelListener(0),
	relay1duration(0),
	relay1delay(0),
	relay2duration(0),
	relay2delay(0),
	relay3duration(0),
	relay3delay(0),
	ammount(0),
	setLengthActual(0)

{

}

void Model::tick()
{
	extern QueueHandle_t qhReportToTouchGFX;
	qPackage_report_t receivedPackage;
	BaseType_t xStatus = xQueuePeek(qhReportToTouchGFX, &receivedPackage,0);
	if(xStatus == pdPASS){
		setLengthActual = receivedPackage.setLengthActual_01mm;
		currLength= receivedPackage.currLength_01mm;
		ammount = receivedPackage.ammount;
	}
}


void Model::setRelay1duration(uint32_t duration){
	relay1duration = duration;
}
void Model::setRelay1delay(uint32_t delay){
	relay1delay = delay;
}

void Model::setRelay2duration(uint32_t duration){
	relay2duration = duration;
}
void Model::setRelay2delay(uint32_t delay){
	relay2delay = delay;
}

void Model::setRelay3duration(uint32_t duration){
	relay3duration = duration;
}
void Model::setRelay3delay(uint32_t delay){
	relay3delay = delay;
}

void Model::resetAmmount(void){
	ammount = 0;
	// TODO: send a message to reset counting timer
}
void Model::enableCutting(bool enable){
	cuttingActive = enable;
	reportToEncoderControlTask();
}

void Model::immCut(){
	immediateCut = true;
	reportToEncoderControlTask();
}

void Model::updateSetLength(uint32_t newLength){
	setLength = newLength;
	reportToEncoderControlTask();
}

void Model::reportToEncoderControlTask(){
    extern QueueHandle_t qhGUItoEncoderControl;
	qPackage_encoderControl_t package = {
		(uint8_t) cuttingActive,
		(uint8_t) immediateCut,
		setLength, // unit: 0.1 mm
		2048, // resolution
		500 // radius unit: 0.1 mm, 500*0.0001m = 0.05m
	};
	BaseType_t xStatus = xQueueSend(qhGUItoEncoderControl, &package, 0); // should be able to put in the queue as it will be emptied immediately
	if(xStatus == pdFAIL){
		while(1);
	}

	immediateCut = false;

}

uint32_t Model::getRelay1duration(){
	return relay1duration;
}
uint32_t Model::getRelay1delay(){
	return relay1delay;
}
uint32_t Model::getRelay2duration(){
	return relay2duration;
}
uint32_t Model::getRelay2delay(){
	return relay2delay;
}
uint32_t Model::getRelay3duration(){
	return relay3duration;
}
uint32_t Model::getRelay3delay(){
	return relay3delay;
}
uint32_t Model::getAmmount(){
	return ammount;
}
bool Model::getCutting(){
	return cuttingActive;
}
uint32_t Model::getSetLength(){
	return setLength;
}
uint32_t Model::getSetLengthActual(){
	return setLengthActual;
}
uint32_t Model::getCurrLength(){
	return currLength;
}















