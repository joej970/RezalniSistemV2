#ifndef SCREEN1MAINCUTTINGVIEW_HPP
#define SCREEN1MAINCUTTINGVIEW_HPP

#include <gui_generated/screen1maincutting_screen/Screen1MainCuttingViewBase.hpp>
#include <gui/screen1maincutting_screen/Screen1MainCuttingPresenter.hpp>

class Screen1MainCuttingView : public Screen1MainCuttingViewBase
{
public:
    Screen1MainCuttingView();
    virtual ~Screen1MainCuttingView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:

private:
    touchgfx::Callback<Screen1MainCuttingView, int16_t> swipeCallback;

    void swipeCallbackHandler(int16_t);
};

#endif // SCREEN1MAINCUTTINGVIEW_HPP
