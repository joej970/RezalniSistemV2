#include <gui/screen1maincutting_screen/Screen1MainCuttingView.hpp>

Screen1MainCuttingView::Screen1MainCuttingView():
	swipeCallback(this, &Screen1MainCuttingView::swipeCallbackHandler)
{
	swipeDetectContainerMainScreen.setAction(swipeCallback);
}

void Screen1MainCuttingView::setupScreen()
{
    Screen1MainCuttingViewBase::setupScreen();
}

void Screen1MainCuttingView::tearDownScreen()
{
    Screen1MainCuttingViewBase::tearDownScreen();
}

void Screen1MainCuttingView::swipeCallbackHandler(int16_t velocity){
	if(velocity < 0){
		application().gotoScreen2CurrLenScreenSlideTransitionEast();
	}
}
