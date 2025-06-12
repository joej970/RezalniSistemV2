#include <gui/screen10setorigin_screen/Screen10SetOriginView.hpp>

Screen10SetOriginView::Screen10SetOriginView()
{

}

void Screen10SetOriginView::setupScreen()
{
	digitSelector_4bits_x0.setCurrentValue(presenter->getOrigin_X0());
    digitSelector_4bits_y0.setCurrentValue(presenter->getOrigin_Y0());
    popUpUartConsoleGRBL.setModel(presenter->getModel());
    Screen10SetOriginViewBase::setupScreen();
}

void Screen10SetOriginView::tearDownScreen()
{
    presenter->setOrigin_X0(digitSelector_4bits_x0.getCurrentValue());
	presenter->setOrigin_Y0(digitSelector_4bits_y0.getCurrentValue());

	presenter->writeToEEPROM(0); // slot 0 for LAST settings

	Screen10SetOriginViewBase::tearDownScreen();
}

void Screen10SetOriginView::handleTickEvent(){

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
