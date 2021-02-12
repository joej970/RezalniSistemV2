#ifndef SCREEN5RELAY3SETUPPRESENTER_HPP
#define SCREEN5RELAY3SETUPPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Screen5Relay3SetupView;

class Screen5Relay3SetupPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen5Relay3SetupPresenter(Screen5Relay3SetupView& v);

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

    virtual ~Screen5Relay3SetupPresenter() {};

    void forwardRelayDuration(uint32_t duration){
    	model->setRelay3duration(duration);
    }
    void forwardRelayDelay(uint32_t delay){
    	model->setRelay3delay(delay);
    }

    uint32_t fetchRelayDuration(){
    	return model->getRelay3duration();
    }
    uint32_t fetchRelayDelay(){
    	return model->getRelay3delay();
    }

private:
    Screen5Relay3SetupPresenter();

    Screen5Relay3SetupView& view;
};

#endif // SCREEN5RELAY3SETUPPRESENTER_HPP
