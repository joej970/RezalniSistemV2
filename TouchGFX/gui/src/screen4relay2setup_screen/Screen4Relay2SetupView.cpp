#include <gui/screen4relay2setup_screen/Screen4Relay2SetupView.hpp>

Screen4Relay2SetupView::Screen4Relay2SetupView()
{

}

void Screen4Relay2SetupView::setupScreen()
{
	uint32_t delay = presenter->fetchRelayDelay();
	uint32_t duration = presenter->fetchRelayDuration();
	digitSelectorDelay.setCurrentValue(delay);
	digitSelectorDuration.setCurrentValue(duration);
    Screen4Relay2SetupViewBase::setupScreen();
}

void Screen4Relay2SetupView::tearDownScreen()
{
    Screen4Relay2SetupViewBase::tearDownScreen();
}

void Screen4Relay2SetupView::saveData()
{
	uint32_t delay = digitSelectorDelay.getCurrentValue();
	uint32_t duration = digitSelectorDuration.getCurrentValue();
    presenter->forwardRelayDelay(delay);
    presenter->forwardRelayDuration(duration);
}
