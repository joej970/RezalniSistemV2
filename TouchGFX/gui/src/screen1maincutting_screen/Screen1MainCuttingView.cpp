#include <gui/screen1maincutting_screen/Screen1MainCuttingView.hpp>

Screen1MainCuttingView::Screen1MainCuttingView():
	swipeCallback(this, &Screen1MainCuttingView::swipeCallbackHandler),
	flexButtonTouchedCallback(this, &Screen1MainCuttingView::flexButtonTouchedHandler)
{
	swipeDetectContainerMainScreen.setAction(swipeCallback);
}

void Screen1MainCuttingView::setupScreen()
{
	relayWidget1.setId(1);
	relayWidget1.setDelay(presenter->fetchRelay1delay());
	relayWidget1.setDuration(presenter->fetchRelay1duration());

	relayWidget2.setId(2);
	relayWidget2.setDelay(presenter->fetchRelay2delay());
	relayWidget2.setDuration(presenter->fetchRelay2duration());

	relayWidget3.setId(3);
	relayWidget3.setDelay(presenter->fetchRelay3delay());
	relayWidget3.setDuration(presenter->fetchRelay3duration());

	flexButtonAmount.setClickAction(flexButtonTouchedCallback);

	releaseValidAfter = HAL_GetTick();

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

void Screen1MainCuttingView::flexButtonTouchedHandler(const ImageButtonStyle< touchgfx::TouchButtonTrigger >& button, const ClickEvent& evt){
	if(&button == &flexButtonAmount){
		if(evt.getType() == ClickEvent::PRESSED){
			//start timeout
			releaseValidAfter = HAL_GetTick() + 1000;
		}
		else if(evt.getType() == ClickEvent::RELEASED){
			//check if already valid
			if(HAL_GetTick() > releaseValidAfter){
				uint32_t timeReached = HAL_GetTick();
 				releaseValidAfter = timeReached; //ensure new value cannot be reached
			}else{
				uint32_t timeReached2 = HAL_GetTick();
 				releaseValidAfter = timeReached2; //ensure new value cannot be reached
			}
		}
	}
}
