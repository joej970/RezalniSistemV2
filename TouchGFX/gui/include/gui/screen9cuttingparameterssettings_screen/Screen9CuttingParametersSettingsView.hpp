#ifndef SCREEN9CUTTINGPARAMETERSSETTINGSVIEW_HPP
#define SCREEN9CUTTINGPARAMETERSSETTINGSVIEW_HPP

#include <gui_generated/screen9cuttingparameterssettings_screen/Screen9CuttingParametersSettingsViewBase.hpp>
#include <gui/screen9cuttingparameterssettings_screen/Screen9CuttingParametersSettingsPresenter.hpp>

class Screen9CuttingParametersSettingsView : public Screen9CuttingParametersSettingsViewBase
{
public:
    Screen9CuttingParametersSettingsView();
    virtual ~Screen9CuttingParametersSettingsView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    void handleTickEvent();

   void load3();

   void save3();

   void save2();

   void load2();

   void save1();

   void load1();

   void updateAndGoBack();

protected:
};

#endif // SCREEN9CUTTINGPARAMETERSSETTINGSVIEW_HPP
