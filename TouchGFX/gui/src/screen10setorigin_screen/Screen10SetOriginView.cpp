#include <gui/screen10setorigin_screen/Screen10SetOriginView.hpp>

Screen10SetOriginView::Screen10SetOriginView()
{

}

void Screen10SetOriginView::setupScreen()
{
	digitSelector_4bits_x0.setCurrentValue(presenter->getOrigin_X0());
    digitSelector_4bits_y0.setCurrentValue(presenter->getOrigin_Y0());
    Screen10SetOriginViewBase::setupScreen();
}

void Screen10SetOriginView::tearDownScreen()
{
    presenter->setOrigin_X0(digitSelector_4bits_x0.getCurrentValue());
	presenter->setOrigin_Y0(digitSelector_4bits_y0.getCurrentValue());

	presenter->writeToEEPROM(0); // slot 0 for LAST settings

	Screen10SetOriginViewBase::tearDownScreen();
}
