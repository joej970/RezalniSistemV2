#include <gui/screen3relay1setup_screen/Screen3Relay1SetupView.hpp>

Screen3Relay1SetupView::Screen3Relay1SetupView()
{
	uint32_t currentValue = 24678;
	digitSelectorDuration.setToCurrentValue(currentValue);
}

void Screen3Relay1SetupView::setupScreen()
{
    Screen3Relay1SetupViewBase::setupScreen();
}

void Screen3Relay1SetupView::tearDownScreen()
{
    Screen3Relay1SetupViewBase::tearDownScreen();
}
