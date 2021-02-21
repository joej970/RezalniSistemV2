#include <gui/screen6lengthsetup_screen/Screen6LengthSetupView.hpp>

Screen6LengthSetupView::Screen6LengthSetupView()
{

}

void Screen6LengthSetupView::setupScreen()
{
	uint32_t length = presenter->fetchSetLength();
	digitSelectorLength.setCurrentValue(length);
    Screen6LengthSetupViewBase::setupScreen();
}

void Screen6LengthSetupView::tearDownScreen()
{
	saveData();
    Screen6LengthSetupViewBase::tearDownScreen();
}

void Screen6LengthSetupView::saveData()
{
	uint32_t newLength = digitSelectorLength.getCurrentValue();
    presenter->forwardSetLength(newLength);
}
