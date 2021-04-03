#ifndef SCREEN1MAINCUTTINGVIEW_HPP
#define SCREEN1MAINCUTTINGVIEW_HPP

#include <gui_generated/screen1maincutting_screen/Screen1MainCuttingViewBase.hpp>
#include <gui/screen1maincutting_screen/Screen1MainCuttingPresenter.hpp>
#include "stm32f7xx_hal.h"

extern const char* eepromStatus_strings[];

class Screen1MainCuttingView : public Screen1MainCuttingViewBase
{
public:
    Screen1MainCuttingView();
    virtual ~Screen1MainCuttingView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();

    void flexButtonTouchedHandler(const ImageButtonStyle< touchgfx::TouchButtonTrigger >& button, const ClickEvent& evt);
protected:

private:
    touchgfx::Callback<Screen1MainCuttingView, int16_t> swipeCallback;
    Callback<Screen1MainCuttingView, const ImageButtonStyle< touchgfx::TouchButtonTrigger >&, const ClickEvent& > flexButtonTouchedCallback;


    void swipeCallbackHandler(int16_t);

    uint32_t amountButtonPressedTime;
    uint32_t startStopButtonPressedTime;
};

#endif // SCREEN1MAINCUTTINGVIEW_HPP
