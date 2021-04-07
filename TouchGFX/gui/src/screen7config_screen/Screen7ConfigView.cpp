#include <gui/screen7config_screen/Screen7ConfigView.hpp>


Screen7ConfigView::Screen7ConfigView()
{

}

void Screen7ConfigView::setupScreen()
{
	uint16_t radius = presenter->fetchRadius();
	digitSelectorRadius.setCurrentValue((uint32_t) radius);
    Screen7ConfigViewBase::setupScreen();
}

void Screen7ConfigView::tearDownScreen()
{
	uint16_t radius = (uint16_t) digitSelectorRadius.getCurrentValue();
    presenter->forwardRadius(radius);
    Screen7ConfigViewBase::tearDownScreen();
}


void Screen7ConfigView::setLanguageSI(){
    Texts::setLanguage(SI);
    container.invalidate();
    presenter->saveLanguage(SI);
}

void Screen7ConfigView::setLanguageENG(){
    Texts::setLanguage(ENG);
    container.invalidate();
    presenter->saveLanguage(ENG);
}
