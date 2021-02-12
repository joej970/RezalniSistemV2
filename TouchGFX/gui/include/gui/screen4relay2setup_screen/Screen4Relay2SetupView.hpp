#ifndef SCREEN4RELAY2SETUPVIEW_HPP
#define SCREEN4RELAY2SETUPVIEW_HPP

#include <gui_generated/screen4relay2setup_screen/Screen4Relay2SetupViewBase.hpp>
#include <gui/screen4relay2setup_screen/Screen4Relay2SetupPresenter.hpp>

class Screen4Relay2SetupView : public Screen4Relay2SetupViewBase
{
public:
    Screen4Relay2SetupView();
    virtual ~Screen4Relay2SetupView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    void saveData();
protected:
};

#endif // SCREEN4RELAY2SETUPVIEW_HPP
