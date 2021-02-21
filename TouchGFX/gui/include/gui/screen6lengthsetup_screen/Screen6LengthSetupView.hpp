#ifndef SCREEN6LENGTHSETUPVIEW_HPP
#define SCREEN6LENGTHSETUPVIEW_HPP

#include <gui_generated/screen6lengthsetup_screen/Screen6LengthSetupViewBase.hpp>
#include <gui/screen6lengthsetup_screen/Screen6LengthSetupPresenter.hpp>

class Screen6LengthSetupView : public Screen6LengthSetupViewBase
{
public:
    Screen6LengthSetupView();
    virtual ~Screen6LengthSetupView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    void saveData();
protected:
};

#endif // SCREEN6LENGTHSETUPVIEW_HPP
