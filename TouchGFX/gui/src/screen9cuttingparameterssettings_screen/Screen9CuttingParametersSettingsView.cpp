#include <gui/screen9cuttingparameterssettings_screen/Screen9CuttingParametersSettingsView.hpp>

Screen9CuttingParametersSettingsView::Screen9CuttingParametersSettingsView()
{

}

void Screen9CuttingParametersSettingsView::setupScreen()
{
    digitSelector_4bits_alpha.setCurrentValue(presenter->getAlpha());
    digitSelector_4bits_beta.setCurrentValue(presenter->getBeta());
    digitSelector_4bits_width.setCurrentValue(presenter->getWidth());
    digitSelector_4bits_feedrate.setCurrentValue(presenter->getFeedrate());
    Screen9CuttingParametersSettingsViewBase::setupScreen();
}

void Screen9CuttingParametersSettingsView::tearDownScreen()
{

    presenter->setAlpha(digitSelector_4bits_alpha.getCurrentValue());
	presenter->setBeta(digitSelector_4bits_beta.getCurrentValue());
	presenter->setWidth(digitSelector_4bits_width.getCurrentValue());
    presenter->setFeedrate(digitSelector_4bits_feedrate.getCurrentValue());

    presenter->writeToEEPROM(0); // slot 0 for LAST settings

    presenter->reportToGRBLControlTask();

    Screen9CuttingParametersSettingsViewBase::tearDownScreen();
}



void Screen9CuttingParametersSettingsView::save3()
{
    presenter->setAlpha(digitSelector_4bits_alpha.getCurrentValue());
	presenter->setBeta(digitSelector_4bits_beta.getCurrentValue());
	presenter->setWidth(digitSelector_4bits_width.getCurrentValue());
    presenter->setFeedrate(digitSelector_4bits_feedrate.getCurrentValue());

    presenter->writeToEEPROM(3);
}

void Screen9CuttingParametersSettingsView::save2()
{
    presenter->setAlpha(digitSelector_4bits_alpha.getCurrentValue());
	presenter->setBeta(digitSelector_4bits_beta.getCurrentValue());
	presenter->setWidth(digitSelector_4bits_width.getCurrentValue());
    presenter->setFeedrate(digitSelector_4bits_feedrate.getCurrentValue());

    presenter->writeToEEPROM(2);
}

void Screen9CuttingParametersSettingsView::save1()
{
    presenter->setAlpha(digitSelector_4bits_alpha.getCurrentValue());
	presenter->setBeta(digitSelector_4bits_beta.getCurrentValue());
	presenter->setWidth(digitSelector_4bits_width.getCurrentValue());
    presenter->setFeedrate(digitSelector_4bits_feedrate.getCurrentValue());

    presenter->writeToEEPROM(1);
}

void Screen9CuttingParametersSettingsView::load3()
{
	presenter->updateLaserParamsFromEEPROM(3);
    digitSelector_4bits_alpha.setCurrentValue(presenter->getAlpha());
    digitSelector_4bits_beta.setCurrentValue(presenter->getBeta());
    digitSelector_4bits_width.setCurrentValue(presenter->getWidth());
    digitSelector_4bits_feedrate.setCurrentValue(presenter->getFeedrate());
}

void Screen9CuttingParametersSettingsView::load2()
{

	presenter->updateLaserParamsFromEEPROM(2);
    digitSelector_4bits_alpha.setCurrentValue(presenter->getAlpha());
    digitSelector_4bits_beta.setCurrentValue(presenter->getBeta());
    digitSelector_4bits_width.setCurrentValue(presenter->getWidth());
    digitSelector_4bits_feedrate.setCurrentValue(presenter->getFeedrate());
}

void Screen9CuttingParametersSettingsView::load1()
{

//
//	uint16_t prev_alpha = presenter->getAlpha()  ;
//	uint16_t prev_beta = presenter->getBeta()   ;
//	uint16_t prev_width = presenter->getWidth()  ;
//	uint16_t prev_feedrate = presenter->getFeedrate();

	presenter->updateLaserParamsFromEEPROM(1);
//       digitSelector_4bits_alpha.setCurrentValue(presenter->getAlpha()   );
//        digitSelector_4bits_beta.setCurrentValue(presenter->getBeta()    );
//       digitSelector_4bits_width.setCurrentValue(presenter->getWidth()   );
//    digitSelector_4bits_feedrate.setCurrentValue(presenter->getFeedrate());


//	uint16_t after_alpha   = presenter->getAlpha()  ;
//	uint16_t after_beta    = presenter->getBeta()   ;
//	uint16_t after_width   = presenter->getWidth()  ;
//	uint16_t after_feedrate = presenter->getFeedrate();
//
//	if(prev_alpha == after_alpha){
//		prev_alpha = 0;
//	}
//	(void)after_alpha;   (void)prev_alpha;
//	(void)after_beta  ;  (void)prev_beta  ;
//	(void)after_width  ; (void)prev_width  ;
//	(void)after_feedrate;(void)prev_feedrate;
}

void Screen9CuttingParametersSettingsView::updateAndGoBack()
{

  // update values in model
	// update is done in: Screen9CuttingParametersSettingsView::tearDownScreen()

	application().gotoScreen8GRBLcontrolScreenSlideTransitionWest();

}

void Screen9CuttingParametersSettingsView::handleTickEvent(){

	if(presenter->shouldUpdateLaserParams()){
		digitSelector_4bits_alpha.setCurrentValue(presenter->getAlpha());
		digitSelector_4bits_beta.setCurrentValue(presenter->getBeta());
		digitSelector_4bits_width.setCurrentValue(presenter->getWidth());
		digitSelector_4bits_feedrate.setCurrentValue(presenter->getFeedrate());
	}

}
