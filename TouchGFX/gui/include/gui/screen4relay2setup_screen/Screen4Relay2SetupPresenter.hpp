#ifndef SCREEN4RELAY2SETUPPRESENTER_HPP
#define SCREEN4RELAY2SETUPPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Screen4Relay2SetupView;

class Screen4Relay2SetupPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen4Relay2SetupPresenter(Screen4Relay2SetupView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~Screen4Relay2SetupPresenter() {};

private:
    Screen4Relay2SetupPresenter();

    Screen4Relay2SetupView& view;
};

#endif // SCREEN4RELAY2SETUPPRESENTER_HPP
