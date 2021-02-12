#ifndef SCREEN5RELAY3SETUPVIEW_HPP
#define SCREEN5RELAY3SETUPVIEW_HPP

#include <gui_generated/screen5relay3setup_screen/Screen5Relay3SetupViewBase.hpp>
#include <gui/screen5relay3setup_screen/Screen5Relay3SetupPresenter.hpp>

class Screen5Relay3SetupView : public Screen5Relay3SetupViewBase
{
public:
    Screen5Relay3SetupView();
    virtual ~Screen5Relay3SetupView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    void saveData();
protected:
};

#endif // SCREEN5RELAY3SETUPVIEW_HPP
