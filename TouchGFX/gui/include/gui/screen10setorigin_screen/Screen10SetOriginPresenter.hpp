#ifndef SCREEN10SETORIGINPRESENTER_HPP
#define SCREEN10SETORIGINPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Screen10SetOriginView;

class Screen10SetOriginPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen10SetOriginPresenter(Screen10SetOriginView& v);

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

    virtual ~Screen10SetOriginPresenter() {};

    uint16_t getOrigin_X0(){return model->getOrigin_X0();}
    uint16_t getOrigin_Y0(){return model->getOrigin_Y0();}

    void setOrigin_X0(uint16_t value){model->setOrigin_X0(value);}
    void setOrigin_Y0(uint16_t value){model->setOrigin_Y0(value);}

    void writeToEEPROM(uint8_t slot){model->writeLaserParamsToEEPROM(slot);}

private:
    Screen10SetOriginPresenter();

    Screen10SetOriginView& view;
};

#endif // SCREEN10SETORIGINPRESENTER_HPP
