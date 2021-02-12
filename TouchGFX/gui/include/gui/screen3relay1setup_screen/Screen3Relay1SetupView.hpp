#ifndef SCREEN3RELAY1SETUPVIEW_HPP
#define SCREEN3RELAY1SETUPVIEW_HPP

#include <gui_generated/screen3relay1setup_screen/Screen3Relay1SetupViewBase.hpp>
#include <gui/screen3relay1setup_screen/Screen3Relay1SetupPresenter.hpp>

class Screen3Relay1SetupView : public Screen3Relay1SetupViewBase
{
public:
    Screen3Relay1SetupView();
    virtual ~Screen3Relay1SetupView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    void saveData();
protected:

};

#endif // SCREEN3RELAY1SETUPVIEW_HPP
