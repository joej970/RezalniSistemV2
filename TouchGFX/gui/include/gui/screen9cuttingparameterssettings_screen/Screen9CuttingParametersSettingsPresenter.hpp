#ifndef SCREEN9CUTTINGPARAMETERSSETTINGSPRESENTER_HPP
#define SCREEN9CUTTINGPARAMETERSSETTINGSPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Screen9CuttingParametersSettingsView;

class Screen9CuttingParametersSettingsPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen9CuttingParametersSettingsPresenter(Screen9CuttingParametersSettingsView& v);

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

    virtual ~Screen9CuttingParametersSettingsPresenter() {};

    void setAlpha(uint16_t angle){model->setAlpha(angle);}
    void   setBeta(uint16_t beta){model->setBeta(beta);}
    void setFeedrate(uint16_t fr){model->setFeedrate(fr);}
    void setWidth(uint16_t width){model->setWidth(width);}
    void setAlphaCutEnable(uint8_t en){model->setAlphaCutEnable(en);}

    void writeToEEPROM(uint8_t slot){model->writeLaserParamsToEEPROM(slot);}
    void updateLaserParamsFromEEPROM(uint8_t slot){model->updateLaserParamsFromEEPROM(slot);}
    bool shouldUpdateLaserParams(){return model->shouldUpdateLaserParams();}

    void reportToGRBLControlTask(){model->reportToGRBLControlTask();}

    uint16_t getAlpha(){return model->getAlpha();}
    uint16_t getBeta(){return model->getBeta();}
    uint16_t getFeedrate(){return model->getFeedrate();}
    uint16_t getWidth(){return model->getWidth();}
    uint8_t getAlphaCutEnable(){return model->getAlphaCutEnable();}

private:
    Screen9CuttingParametersSettingsPresenter();

    Screen9CuttingParametersSettingsView& view;
};

#endif // SCREEN9CUTTINGPARAMETERSSETTINGSPRESENTER_HPP
