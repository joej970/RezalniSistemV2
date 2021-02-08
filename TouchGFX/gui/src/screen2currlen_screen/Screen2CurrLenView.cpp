#include <gui/screen2currlen_screen/Screen2CurrLenView.hpp>

Screen2CurrLenView::Screen2CurrLenView():
	swipeCallback(this, &Screen2CurrLenView::swipeCallbackHandler)
{
	swipeDetectContainerCurrLenScreen.setAction(swipeCallback);
}

void Screen2CurrLenView::setupScreen()
{
    Screen2CurrLenViewBase::setupScreen();
}

void Screen2CurrLenView::tearDownScreen()
{
    Screen2CurrLenViewBase::tearDownScreen();
}

void Screen2CurrLenView::swipeCallbackHandler(int16_t velocity){
	if (velocity > 0){
		application().gotoScreen1MainCuttingScreenSlideTransitionWest();
	}

}

