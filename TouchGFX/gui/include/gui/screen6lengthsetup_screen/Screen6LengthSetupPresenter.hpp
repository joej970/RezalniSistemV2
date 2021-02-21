#ifndef SCREEN6LENGTHSETUPPRESENTER_HPP
#define SCREEN6LENGTHSETUPPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Screen6LengthSetupView;

class Screen6LengthSetupPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen6LengthSetupPresenter(Screen6LengthSetupView& v);

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

    virtual ~Screen6LengthSetupPresenter() {};

    uint32_t fetchSetLength(){
    	return model->getSetLength();
    }

    void forwardSetLength(uint32_t newLength){
    	model->updateSetLength(newLength);
    }

private:
    Screen6LengthSetupPresenter();

    Screen6LengthSetupView& view;
};

#endif // SCREEN6LENGTHSETUPPRESENTER_HPP
