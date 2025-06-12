

#include <gui/screen8grblcontrol_screen/Screen8GRBLcontrolView.hpp>
#include "BitmapDatabase.hpp"



Screen8GRBLcontrolView::Screen8GRBLcontrolView():
	swipeCallback(this, &Screen8GRBLcontrolView::swipeCallbackHandler)
{
	swipeDetectContainer1.setAction(swipeCallback);
}

void Screen8GRBLcontrolView::setupScreen()
{

	popUpUartConsoleGRBL.setModel(presenter->getModel());
	grblHomingRequest.setModel(presenter->getModel());

//    lastGRBLStatus_temp = (Screen8GRBLcontrolView::grblConn_t)presenter->getGRBLconnStatus();
	switch(presenter->getGRBLconnStatus()){
		case NOT_CONNECTED:
			grblConnectionStatusButton.setBitmaps(Bitmap(BITMAP_RADIO_ROUND_BUTTON_ACTIVE_RED_ID), Bitmap(BITMAP_RADIO_ROUND_BUTTON_ACTIVE_RED_ID));
			break;
		case PENDING:
			grblConnectionStatusButton.setBitmaps(Bitmap(BITMAP_RADIO_ROUND_BUTTON_ACTIVE_ORANGE_ID), Bitmap(BITMAP_RADIO_ROUND_BUTTON_ACTIVE_ORANGE_ID));
			break;
		case CONNECTED:
			grblConnectionStatusButton.setBitmaps(Bitmap(BITMAP_RADIO_ROUND_BUTTON_ACTIVE_GREEN_ID), Bitmap(BITMAP_RADIO_ROUND_BUTTON_ACTIVE_GREEN_ID));
			break;
	}
	grblConnectionStatusButton.invalidate();

    Screen8GRBLcontrolViewBase::setupScreen();
}

void Screen8GRBLcontrolView::tearDownScreen()
{
    Screen8GRBLcontrolViewBase::tearDownScreen();
}



void Screen8GRBLcontrolView::handleTickEvent(){

    Bitmap newBitmap;
    switch(presenter->getGRBLconnStatus()) {
        case NOT_CONNECTED:
            newBitmap = Bitmap(BITMAP_RADIO_ROUND_BUTTON_ACTIVE_RED_ID);
            break;
        case PENDING:
            newBitmap = Bitmap(BITMAP_RADIO_ROUND_BUTTON_ACTIVE_ORANGE_ID);
            break;
        case CONNECTED:
            newBitmap = Bitmap(BITMAP_RADIO_ROUND_BUTTON_ACTIVE_GREEN_ID);
            break;
    }

    if (grblConnectionStatusButton.getCurrentlyDisplayedBitmap() != newBitmap) {
        grblConnectionStatusButton.setBitmaps(newBitmap, newBitmap);
        grblConnectionStatusButton.invalidate();
//		printf("Status has changed from %d to %d \n", lastGRBLStatus_temp, presenter->getGRBLconnStatus());
    }


	Unicode::snprintf(textAreaFeedRateBuffer, TEXTAREAFEEDRATE_SIZE, "%d", presenter->getFeedrate());
	textAreaFeedRate.invalidate();

	float width = 0.1*(float)presenter->getWidth();
	Unicode::snprintfFloat(textAreaWidthBuffer, TEXTAREAWIDTH_SIZE, "%0.1f", width);
	textAreaWidth.invalidate();

	float alpha = 0.1*(float)presenter->getAlpha();
	Unicode::snprintfFloat(textAreaAlphaBuffer, TEXTAREAALPHA_SIZE, "%0.1f", alpha);
	textAreaAlpha.invalidate();

	float beta = 0.1*(float)presenter->getBeta();
	Unicode::snprintfFloat(textAreaBetaBuffer, TEXTAREABETA_SIZE, "%0.1f", beta);
	textAreaBeta.invalidate();


	switch(presenter->fetchLastStatus()){
		case OP_OK:
			break;
		case SET_LENGTH_VALID:
			break;
		case SET_LENGTH_TRIMMED:
			break;
		case RELAY_DELAY_OF:
			break;
		case RELAY_DURATION_OF:
			break;
		case RELAY_DEACTIVATED:
			break;
		case SETTINGS_LOAD_ERR:
			break;
		case SETTINGS_SAVE_ERR:
			break;
		case UART_TX_NOT_OKED:
			presenter->resetLastStatus();
			popUpWindowMain.setVisible(true);
			popUpWindowMain.setTextWithMessage(T_STATUSMSG_SETTINGS_UART_TX_NOT_OKED, presenter->fetchMessage());
			popUpWindowMain.invalidate();
			break;
		case UART_TX_ERR:
			presenter->resetLastStatus();
			popUpWindowMain.setVisible(true);
			popUpWindowMain.setTextWithMessage(T_STATUSMSG_SETTINGS_UART_TX_ERR, presenter->fetchMessage());
			popUpWindowMain.invalidate();
			break;
		case ORIGIN_UPDATED:
			presenter->resetLastStatus();
			popUpWindowMain.setVisible(true);
			popUpWindowMain.setText(T_STATUSMSG_ORIGIN_UPDATED);
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



void Screen8GRBLcontrolView::swipeCallbackHandler(int16_t velocity) {
	if (velocity < 0) {
		application().gotoScreen1MainCuttingScreenSlideTransitionEast();
	}

}
