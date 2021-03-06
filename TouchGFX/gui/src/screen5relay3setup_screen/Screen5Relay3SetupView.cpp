#include <gui/screen5relay3setup_screen/Screen5Relay3SetupView.hpp>

Screen5Relay3SetupView::Screen5Relay3SetupView()
{

}

void Screen5Relay3SetupView::setupScreen()
{
	uint32_t delay = presenter->fetchRelayDelay();
	uint32_t duration = presenter->fetchRelayDuration();
	digitSelectorDelay.setCurrentValue(delay);
	digitSelectorDuration.setCurrentValue(duration);
    Screen5Relay3SetupViewBase::setupScreen();
}

void Screen5Relay3SetupView::tearDownScreen()
{
    Screen5Relay3SetupViewBase::tearDownScreen();
}

void Screen5Relay3SetupView::saveData()
{
	uint32_t delay = digitSelectorDelay.getCurrentValue();
	uint32_t duration = digitSelectorDuration.getCurrentValue();
    presenter->forwardRelayDelay(delay);
    presenter->forwardRelayDuration(duration);
}
