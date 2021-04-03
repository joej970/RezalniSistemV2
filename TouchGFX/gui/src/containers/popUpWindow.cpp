#include <gui/containers/popUpWindow.hpp>


popUpWindow::popUpWindow()
{

}

void popUpWindow::initialize()
{
    popUpWindowBase::initialize();
    statusText.setWideTextAction(WIDE_TEXT_WORDWRAP_ELLIPSIS_AFTER_SPACE);
}



void popUpWindow::hidePopUp(){
	this->setVisible(false);
	this->invalidate();
//	statusText.invalidate();
}

void popUpWindow::setText(TEXTS textId){
	statusText.setTypedText(touchgfx::TypedText(textId));
	statusText.resizeHeightToCurrentText();
	statusIdText.setVisible(false);
	statusIdText.invalidate();
}

void popUpWindow::setTextWithMessage(TEXTS textId, const char* message){
	statusText.setTypedText(touchgfx::TypedText(textId));
	statusText.resizeHeightToCurrentText();
	statusIdText.setVisible(true);
	Unicode::strncpy(statusIdTextBuffer, message, Unicode::strlen(message));
	statusIdText.invalidate();
}
