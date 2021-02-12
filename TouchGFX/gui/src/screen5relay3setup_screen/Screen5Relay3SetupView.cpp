#include <gui/screen5relay3setup_screen/Screen5Relay3SetupView.hpp>

Screen5Relay3SetupView::Screen5Relay3SetupView()
{

}

void Screen5Relay3SetupView::setupScreen()
{
	uint32_t duration = presenter->fetchRelayDuration();
	uint32_t delay = presenter->fetchRelayDelay();
	digitSelectorDuration.setCurrentValue(duration);
	digitSelectorDelay.setCurrentValue(delay);
    Screen5Relay3SetupViewBase::setupScreen();
}

void Screen5Relay3SetupView::tearDownScreen()
{
    Screen5Relay3SetupViewBase::tearDownScreen();
}

void Screen5Relay3SetupView::saveData()
{
	uint32_t duration = digitSelectorDuration.getCurrentValue();
	uint32_t delay = digitSelectorDelay.getCurrentValue();
    presenter->forwardRelayDuration(duration);
    presenter->forwardRelayDelay(delay);
}
