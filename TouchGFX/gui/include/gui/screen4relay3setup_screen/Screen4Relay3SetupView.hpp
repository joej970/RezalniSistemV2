#ifndef SCREEN4RELAY3SETUPVIEW_HPP
#define SCREEN4RELAY3SETUPVIEW_HPP

#include <gui_generated/screen4relay3setup_screen/Screen4Relay3SetupViewBase.hpp>
#include <gui/screen4relay3setup_screen/Screen4Relay3SetupPresenter.hpp>

class Screen4Relay3SetupView : public Screen4Relay3SetupViewBase
{
public:
    Screen4Relay3SetupView();
    virtual ~Screen4Relay3SetupView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // SCREEN4RELAY3SETUPVIEW_HPP
