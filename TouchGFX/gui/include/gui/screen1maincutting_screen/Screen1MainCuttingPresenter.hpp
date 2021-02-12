#ifndef SCREEN1MAINCUTTINGPRESENTER_HPP
#define SCREEN1MAINCUTTINGPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Screen1MainCuttingView;

class Screen1MainCuttingPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen1MainCuttingPresenter(Screen1MainCuttingView& v);

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

    virtual ~Screen1MainCuttingPresenter() {};


    uint32_t fetchRelay1duration(){
    	return model->getRelay1duration();
    }
    uint32_t fetchRelay1delay(){
    	return model->getRelay1delay();
    }

    uint32_t fetchRelay2duration(){
    	return model->getRelay2duration();
    }
    uint32_t fetchRelay2delay(){
    	return model->getRelay2delay();
    }

    uint32_t fetchRelay3duration(){
    	return model->getRelay3duration();
    }
    uint32_t fetchRelay3delay(){
    	return model->getRelay3delay();
    }



private:
    Screen1MainCuttingPresenter();

    Screen1MainCuttingView& view;
};

#endif // SCREEN1MAINCUTTINGPRESENTER_HPP
