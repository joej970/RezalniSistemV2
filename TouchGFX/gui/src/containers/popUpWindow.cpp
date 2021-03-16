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
//	statusText.setTypedText(touchgfx::TypedText(T_STATUSMSG_DURATION_OF));
	statusText.setTypedText(touchgfx::TypedText(textId));
	statusText.resizeHeightToCurrentText();
	//statusText.invalidate();
}
