#ifndef SCREEN7CONFIGVIEW_HPP
#define SCREEN7CONFIGVIEW_HPP

#include <gui_generated/screen7config_screen/Screen7ConfigViewBase.hpp>
#include <gui/screen7config_screen/Screen7ConfigPresenter.hpp>

class Screen7ConfigView : public Screen7ConfigViewBase
{
public:
    Screen7ConfigView();
    virtual ~Screen7ConfigView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // SCREEN7CONFIGVIEW_HPP
