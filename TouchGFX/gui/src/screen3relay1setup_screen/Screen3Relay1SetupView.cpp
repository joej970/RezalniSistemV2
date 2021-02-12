#include <gui/screen3relay1setup_screen/Screen3Relay1SetupView.hpp>

Screen3Relay1SetupView::Screen3Relay1SetupView()
{

}

void Screen3Relay1SetupView::setupScreen()
{
	uint32_t duration = presenter->fetchRelayDuration();
	uint32_t delay = presenter->fetchRelayDelay();
	digitSelectorDuration.setCurrentValue(duration);
	digitSelectorDelay.setCurrentValue(delay);
    Screen3Relay1SetupViewBase::setupScreen();
}

void Screen3Relay1SetupView::tearDownScreen()
{
    Screen3Relay1SetupViewBase::tearDownScreen();
}

void Screen3Relay1SetupView::saveData()
{
	uint32_t duration = digitSelectorDuration.getCurrentValue();
	uint32_t delay = digitSelectorDelay.getCurrentValue();
    presenter->forwardRelayDuration(duration);
    presenter->forwardRelayDelay(delay);
}
