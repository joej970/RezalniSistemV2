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
	uint16_t charsCopied = Unicode::strncpy(statusIdTextBuffer, message, STATUSIDTEXT_SIZE);
	printf("popUpWindow: message: %s (%u/%u)\n", message, charsCopied, Unicode::strlen(message) );
//	uint16_t fromUTF8(message, statusIdTextBuffer, Unicode::strlen(message));
	statusIdText.invalidate();
}
