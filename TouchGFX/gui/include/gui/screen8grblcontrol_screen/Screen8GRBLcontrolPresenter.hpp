#ifndef SCREEN8GRBLCONTROLPRESENTER_HPP
#define SCREEN8GRBLCONTROLPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>
#include <vector>

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


//    bool shouldUartConsoleBeVisible();
    std::vector<const char*> fetchUartLineBuffers();

private:
    Screen8GRBLcontrolPresenter();

    Screen8GRBLcontrolView& view;
};

#endif // SCREEN8GRBLCONTROLPRESENTER_HPP
