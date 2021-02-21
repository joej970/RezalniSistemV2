#include <gui/screen1maincutting_screen/Screen1MainCuttingView.hpp>
#include "BitmapDatabase.hpp"

Screen1MainCuttingView::Screen1MainCuttingView() :
	swipeCallback(this, &Screen1MainCuttingView::swipeCallbackHandler),
	flexButtonTouchedCallback(this,&Screen1MainCuttingView::flexButtonTouchedHandler) {
	swipeDetectContainerMainScreen.setAction(swipeCallback);
}

void Screen1MainCuttingView::setupScreen() {
	relayWidget1.setId(1);
	relayWidget1.setDelay(presenter->fetchRelay1delay());
	relayWidget1.setDuration(presenter->fetchRelay1duration());

	relayWidget2.setId(2);
	relayWidget2.setDelay(presenter->fetchRelay2delay());
	relayWidget2.setDuration(presenter->fetchRelay2duration());

	relayWidget3.setId(3);
	relayWidget3.setDelay(presenter->fetchRelay3delay());
	relayWidget3.setDuration(presenter->fetchRelay3duration());

	Unicode::snprintf(textSetLenBuffer, TEXTSETLEN_SIZE, "%d", presenter->fetchSetLength());
	textSetLen.invalidate();

	flexButtonAmount.setClickAction(flexButtonTouchedCallback);
	flexButtonStartStop.setClickAction(flexButtonTouchedCallback);

	amountButtonPressedTime = HAL_GetTick();
	startStopButtonPressedTime  = HAL_GetTick();

	Screen1MainCuttingViewBase::setupScreen();
}

void Screen1MainCuttingView::tearDownScreen() {
	Screen1MainCuttingViewBase::tearDownScreen();
}

void Screen1MainCuttingView::swipeCallbackHandler(int16_t velocity) {
	if (velocity < 0) {
		application().gotoScreen2CurrLenScreenSlideTransitionEast();
	}
}

void Screen1MainCuttingView::flexButtonTouchedHandler(const ImageButtonStyle<touchgfx::TouchButtonTrigger> &button,const ClickEvent &evt) {
	if (&button == &flexButtonAmount) {
		if (evt.getType() == ClickEvent::PRESSED) {
			if(HAL_GetTick() < amountButtonPressedTime + 200 ){
				//double press
				presenter -> initiateImmCut();
				//maybe add some pop up notification
			}
			amountButtonPressedTime = HAL_GetTick();
			circleProgress.setVisible(true);
			circleProgress.invalidate();

		} else if (evt.getType() == ClickEvent::RELEASED) {
			if (HAL_GetTick() > amountButtonPressedTime + 1000) {// LONG PRESS?
				presenter -> resetAmmount();
			}
			circleProgress.setValue(0);
			circleProgress.setVisible(false);
			circleProgress.invalidate();
		}
	} else if (&button == &flexButtonStartStop) {
		if (evt.getType() == ClickEvent::PRESSED) {
			//start timeout
			startStopButtonPressedTime = HAL_GetTick() ;
			if(presenter->isCutting()==false){
				circleProgress.setVisible(true);
				circleProgress.invalidate();
			}
		} else if (evt.getType() == ClickEvent::RELEASED) {
			circleProgress.setValue(0);
			circleProgress.setVisible(false);
			circleProgress.invalidate();
			if(presenter->isCutting()){
				presenter->enableCutting(false);
				flexButtonStartStop.setBitmaps(Bitmap(BITMAP_BUTTON_GREEN_RELEASED_ID), Bitmap(BITMAP_BUTTON_GREEN_PRESSED_ID));
			}else if (HAL_GetTick() > startStopButtonPressedTime + 1000) {
				presenter->enableCutting(true);
				flexButtonStartStop.setBitmaps(Bitmap(BITMAP_BUTTON_RED_RELEASED_ID), Bitmap(BITMAP_BUTTON_RED_PRESSED_ID));
			}

		}
	}
}

void Screen1MainCuttingView::handleTickEvent(){
	if(circleProgress.isVisible()){
		int32_t currentValue = circleProgress.getValue();
	    int max;
	    int min;
	    circleProgress.getRange(min, max);
		if(currentValue != max){
			circleProgress.setValue(currentValue+6);
		}
	}
	Unicode::snprintf(textCurrLenBuffer, TEXTCURRLEN_SIZE, "%d", presenter->fetchCurrLength());
	textCurrLen.invalidate();

}















