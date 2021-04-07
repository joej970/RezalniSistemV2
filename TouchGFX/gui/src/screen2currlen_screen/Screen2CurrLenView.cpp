#include <gui/screen2currlen_screen/Screen2CurrLenView.hpp>

Screen2CurrLenView::Screen2CurrLenView():
	swipeCallback(this, &Screen2CurrLenView::swipeCallbackHandler)
{
	swipeDetectContainerCurrLenScreen.setAction(swipeCallback);
}

void Screen2CurrLenView::setupScreen()
{
    Screen2CurrLenViewBase::setupScreen();
    toggleButtonRelaysActive.forceState( !( (bool) presenter->fetchRelaysActive()) );
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

void Screen2CurrLenView::handleTickEvent(){
	uint32_t amount = presenter->fetchAmount();
	uint32_t setLength = presenter->fetchSetLengthActual(); //0.1 mm
	uint32_t currLength = presenter->fetchCurrLength(); //0.1 mm

	uint32_t totalLength_01m = (amount*setLength+currLength)/1000; // 0.1 mm -> 0.1 m

	Unicode::snprintfFloat(textTotalLengthBuffer, TEXTTOTALLENGTH_SIZE, "%0.1f", 0.1*(float)totalLength_01m);
	textTotalLength.invalidate();

}

void Screen2CurrLenView::toggleRelaysActive()
{
	presenter->toggleRelaysActive();
}

void Screen2CurrLenView::resetLengthMeasurement()
{
	presenter->resetTotalLength();
}

