#ifndef SCREEN8GRBLCONTROLPRESENTER_HPP
#define SCREEN8GRBLCONTROLPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>
#include <vector>
#include "main.h"

using namespace touchgfx;

class Screen8GRBLcontrolView;

class Screen8GRBLcontrolPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen8GRBLcontrolPresenter(Screen8GRBLcontrolView& v);


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

    virtual ~Screen8GRBLcontrolPresenter() {};

    uint16_t getAlpha(){return model->getAlpha();}
    uint16_t getBeta(){return model->getBeta();}
    uint16_t getFeedrate(){return model->getFeedrate();}
    uint16_t getWidth(){return model->getWidth();}

    grblConn_t getGRBLconnStatus(){
    	return model->getGRBLconnStatus();
    }

    statusId_t fetchLastStatus(){
    	return model->getLastStatus();
    }

    char* fetchMessage(){
    	return model->getMessage();
    }

    void resetLastStatus(){
    	model->resetLastStatus();
    }

    std::vector<const char*> fetchUartLineBuffers(){
    	return model->fetchUartLineBuffers();
    }

    void tryToConnectGRBL()
   	{
   		model->tryToConnectGRBL();
   	}

    Model* getModel(){
    	return model;
    }

    bool uartConsoleVisible = false;

    void onConsoleDataUpdated();



private:
    Screen8GRBLcontrolPresenter();

    Screen8GRBLcontrolView& view;
};

#endif // SCREEN8GRBLCONTROLPRESENTER_HPP
