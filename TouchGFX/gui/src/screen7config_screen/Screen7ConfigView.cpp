#include <gui/screen7config_screen/Screen7ConfigView.hpp>


Screen7ConfigView::Screen7ConfigView()
{

}

void Screen7ConfigView::setupScreen()
{
	uint16_t radius = presenter->fetchRadius();
	uint16_t circumference = presenter->fetchCircumference();
	uint16_t resolution = presenter->fetchResolution();

	if(radius == 0){
		useRadiusSwitch.forceState(false);
	}else{
		useRadiusSwitch.forceState(true);
	}

	if(presenter->fetchLaserConsoleActive() > 0){
		laserConsoleActive.forceState(true);
	}else{
		laserConsoleActive.forceState(false);
	}

	digitSelectorRadius.setCurrentValue((uint32_t) radius);
	digitSelectorCircumference.setCurrentValue((uint32_t) circumference);
	digitSelectorResolution.setCurrentValue((uint32_t) resolution);

    Screen7ConfigViewBase::setupScreen();
}

void Screen7ConfigView::tearDownScreen()
{
	uint16_t radius = (uint16_t) digitSelectorRadius.getCurrentValue();
	uint16_t circumference = (uint16_t) digitSelectorCircumference.getCurrentValue();
	uint16_t resolution = (uint16_t) digitSelectorResolution.getCurrentValue();

	if(useRadiusSwitch.getState()){
		circumference = 0;
	}else{
		radius = 0;
	}


	presenter->forwardRadiusCircumferenceResolution(radius, circumference, resolution);

	if(laserConsoleActive.getState()){
		presenter->forwardLaserConsoleActive(1);
	}else{
		presenter->forwardLaserConsoleActive(0);
	}

//    presenter->forwardRadius(radius);
    Screen7ConfigViewBase::tearDownScreen();
}


void Screen7ConfigView::setLanguageSI(){
    Texts::setLanguage(SI);
    container.invalidate();
    presenter->saveLanguage(SI);
}

void Screen7ConfigView::setLanguageENG(){
    Texts::setLanguage(ENG);
    container.invalidate();
    presenter->saveLanguage(ENG);
}
