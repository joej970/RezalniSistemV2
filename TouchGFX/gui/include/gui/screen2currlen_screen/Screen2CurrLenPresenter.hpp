#ifndef SCREEN2CURRLENPRESENTER_HPP
#define SCREEN2CURRLENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Screen2CurrLenView;

class Screen2CurrLenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen2CurrLenPresenter(Screen2CurrLenView& v);

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

    virtual ~Screen2CurrLenPresenter() {};

    uint32_t fetchSetLengthActual(){
    	return model->getSetLengthActual();
    }

    uint32_t fetchCurrLength(){
    	return model->getCurrLength();
    }

    uint32_t fetchAmount(){
    	return model->getAmount();
    }

    uint8_t fetchRelaysActive(){
    	return model->getRelaysActive();
    }

    void toggleRelaysActive(){
    	model->toggleRelaysActive();
    }

    void resetTotalLength(){
    	model->resetAmount();
    	model->resetCurrLength();
    }



private:
    Screen2CurrLenPresenter();

    Screen2CurrLenView& view;
};

#endif // SCREEN2CURRLENPRESENTER_HPP
