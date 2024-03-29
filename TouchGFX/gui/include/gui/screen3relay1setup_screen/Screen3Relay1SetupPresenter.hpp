#ifndef SCREEN3RELAY1SETUPPRESENTER_HPP
#define SCREEN3RELAY1SETUPPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Screen3Relay1SetupView;

class Screen3Relay1SetupPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen3Relay1SetupPresenter(Screen3Relay1SetupView& v);

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

    virtual ~Screen3Relay1SetupPresenter() {};


    void forwardRelayDuration(uint32_t duration){
    	model->setRelay1duration(duration);
    }
    void forwardRelayDelay(uint32_t delay){
    	model->setRelay1delay(delay);
    }

    uint32_t fetchRelayDuration(){
    	return model->getRelay1duration();
    }
    uint32_t fetchRelayDelay(){
    	return model->getRelay1delay();
    }




private:
    Screen3Relay1SetupPresenter();

    Screen3Relay1SetupView& view;
};

#endif // SCREEN3RELAY1SETUPPRESENTER_HPP
