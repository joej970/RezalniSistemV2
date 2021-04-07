#ifndef SCREEN2CURRLENVIEW_HPP
#define SCREEN2CURRLENVIEW_HPP

#include <gui_generated/screen2currlen_screen/Screen2CurrLenViewBase.hpp>
#include <gui/screen2currlen_screen/Screen2CurrLenPresenter.hpp>

class Screen2CurrLenView : public Screen2CurrLenViewBase
{
public:
    Screen2CurrLenView();
    virtual ~Screen2CurrLenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
private:
    touchgfx::Callback<Screen2CurrLenView, int16_t> swipeCallback;

    void swipeCallbackHandler(int16_t);

    void handleTickEvent();

    void toggleRelaysActive();  // Override and implement this function in Screen2CurrLen

    void resetLengthMeasurement();  // Override and implement this function in Screen2CurrLen

};

#endif // SCREEN2CURRLENVIEW_HPP
