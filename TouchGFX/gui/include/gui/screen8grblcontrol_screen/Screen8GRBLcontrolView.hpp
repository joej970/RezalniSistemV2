#ifndef SCREEN8GRBLCONTROLVIEW_HPP
#define SCREEN8GRBLCONTROLVIEW_HPP

#include <gui_generated/screen8grblcontrol_screen/Screen8GRBLcontrolViewBase.hpp>
#include <gui/screen8grblcontrol_screen/Screen8GRBLcontrolPresenter.hpp>

class Screen8GRBLcontrolView : public Screen8GRBLcontrolViewBase
{
public:
    Screen8GRBLcontrolView();
    virtual ~Screen8GRBLcontrolView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:

    touchgfx::Callback<Screen8GRBLcontrolView, int16_t> swipeCallback;

    void swipeCallbackHandler(int16_t);

    void handleTickEvent();


};

#endif // SCREEN8GRBLCONTROLVIEW_HPP
