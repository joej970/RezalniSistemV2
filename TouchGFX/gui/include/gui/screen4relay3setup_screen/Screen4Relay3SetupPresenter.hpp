#ifndef SCREEN4RELAY3SETUPPRESENTER_HPP
#define SCREEN4RELAY3SETUPPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Screen4Relay3SetupView;

class Screen4Relay3SetupPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen4Relay3SetupPresenter(Screen4Relay3SetupView& v);

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

    virtual ~Screen4Relay3SetupPresenter() {};

private:
    Screen4Relay3SetupPresenter();

    Screen4Relay3SetupView& view;
};

#endif // SCREEN4RELAY3SETUPPRESENTER_HPP
