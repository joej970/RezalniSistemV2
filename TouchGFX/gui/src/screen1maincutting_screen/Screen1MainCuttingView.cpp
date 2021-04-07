#include <gui/screen1maincutting_screen/Screen1MainCuttingView.hpp>
#include "BitmapDatabase.hpp"
#include <texts/TextKeysAndLanguages.hpp>


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

//	Unicode::snprintf(textSetLenBuffer, TEXTSETLEN_SIZE, "%d", presenter->fetchSetLength());
//	textSetLen.invalidate();

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
				presenter -> resetAmount();
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
	Unicode::snprintfFloat(textCurrLenBuffer, TEXTCURRLEN_SIZE, "%0.1f", 0.1*(float)presenter->fetchCurrLength());
	textCurrLen.invalidate();
	Unicode::snprintfFloat(textSetLenBuffer, TEXTSETLEN_SIZE, "%0.1f", 0.1*(float)presenter->fetchSetLengthActual());
	textSetLen.invalidate();
	Unicode::snprintf(textAmountBuffer, TEXTAMOUNT_SIZE, "%d", presenter->fetchAmount());
	textAmount.invalidate();

	switch(presenter->fetchLastStatus()){
		case OP_OK:
			break;
		case SET_LENGTH_VALID:
			presenter->resetLastStatus();
			break;
		case SET_LENGTH_TRIMMED:
			presenter->resetLastStatus();
			popUpWindowMain.setVisible(true);
			popUpWindowMain.setText(T_STATUSMSG_SET_LENGTH_TRIMMED);
			popUpWindowMain.invalidate();
//			popUpWindowMain.setText(SET_LENGTH_TRIMMED);
			break;
		case RELAY_DELAY_OF:
			presenter->resetLastStatus();
			popUpWindowMain.setVisible(true);
			popUpWindowMain.setText(T_STATUSMSG_DELAY_OF);
			popUpWindowMain.invalidate();
			break;
		case RELAY_DURATION_OF:
			presenter->resetLastStatus();
			popUpWindowMain.setVisible(true);
			popUpWindowMain.setText(T_STATUSMSG_DURATION_OF);
			popUpWindowMain.invalidate();
			break;
		case RELAY_DEACTIVATED:
			presenter->resetLastStatus();
			popUpWindowMain.setVisible(true);
			popUpWindowMain.setText(T_STATUSMSG_RELAY_DEACTIVATED);
			popUpWindowMain.invalidate();
			break;
		case SETTINGS_LOAD_SUCCESS:
			presenter->resetLastStatus();
			relayWidget1.setDelay(presenter->fetchRelay1delay());
			relayWidget1.setDuration(presenter->fetchRelay1duration());
			relayWidget2.setDelay(presenter->fetchRelay2delay());
			relayWidget2.setDuration(presenter->fetchRelay2duration());
			relayWidget3.setDelay(presenter->fetchRelay3delay());
			relayWidget3.setDuration(presenter->fetchRelay3duration());
			break;
		case SETTINGS_LOAD_ERR:
			presenter->resetLastStatus();
			popUpWindowMain.setVisible(true);
			popUpWindowMain.setText(T_STATUSMSG_SETTINGS_LOAD_ERR);
			popUpWindowMain.invalidate();
			break;
		case SETTINGS_SAVE_ERR:
			presenter->resetLastStatus();
			popUpWindowMain.setVisible(true);
			popUpWindowMain.setTextWithMessage(T_STATUSMSG_SETTINGS_SAVE_ERR, presenter->fetchMessage());
			popUpWindowMain.invalidate();
			break;
		default:
			presenter->resetLastStatus();
			popUpWindowMain.setVisible(true);
			popUpWindowMain.setTextWithMessage(T_STATUSMSG_OTHER_ERR, presenter->fetchMessage());
			popUpWindowMain.invalidate();
			break;

	}



}















