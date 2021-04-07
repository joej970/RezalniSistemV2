#ifndef SCREEN7CONFIGPRESENTER_HPP
#define SCREEN7CONFIGPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>
#include <texts/TextKeysAndLanguages.hpp>

using namespace touchgfx;

class Screen7ConfigView;

class Screen7ConfigPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen7ConfigPresenter(Screen7ConfigView& v);

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

    void saveLanguage(LANGUAGES language){
    	model->saveLanguage(language);
    }
    uint16_t fetchRadius(){
    	return model->getRadius();
    }
    void forwardRadius(uint16_t radius){
    	model->setRadius(radius);
    }

    virtual ~Screen7ConfigPresenter() {};

private:
    Screen7ConfigPresenter();

    Screen7ConfigView& view;
};

#endif // SCREEN7CONFIGPRESENTER_HPP
