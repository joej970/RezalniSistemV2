#include <gui/screen8grblcontrol_screen/Screen8GRBLcontrolView.hpp>


//extern

Screen8GRBLcontrolView::Screen8GRBLcontrolView():
	swipeCallback(this, &Screen8GRBLcontrolView::swipeCallbackHandler)
{
	swipeDetectContainer1.setAction(swipeCallback);
}

void Screen8GRBLcontrolView::setupScreen()
{
    Screen8GRBLcontrolViewBase::setupScreen();
}

void Screen8GRBLcontrolView::tearDownScreen()
{
    Screen8GRBLcontrolViewBase::tearDownScreen();
}



void Screen8GRBLcontrolView::handleTickEvent(){

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


	// update console contents
//	popUpUartConsoleGRBL.setText(presenter->fetchUartBuffer());


	popUpUartConsoleGRBL.setLinesText(presenter->fetchUartLineBuffers());
//	popUpUartConsoleGRBL.setVisible(presenter->shouldUartConsoleBeVisible());
}


void Screen8GRBLcontrolView::swipeCallbackHandler(int16_t velocity) {
	if (velocity < 0) {
		application().gotoScreen1MainCuttingScreenSlideTransitionEast();
	}

}
