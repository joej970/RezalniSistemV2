#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

Model::Model() :
	modelListener(0),
	relay1duration(0),
	relay1delay(0),
	relay2duration(0),
	relay2delay(0),
	relay3duration(0),
	relay3delay(0),
	ammount(0)
{

}

void Model::tick()
{

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
}
void Model::enableCutting(bool enable){
	cuttingActive = enable;
}

void Model::immCut(){

}

void Model::updateSetLength(uint32_t newLength){
	setLength = newLength;
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
uint32_t Model::getCurrLength(){
	return currLength;
}















