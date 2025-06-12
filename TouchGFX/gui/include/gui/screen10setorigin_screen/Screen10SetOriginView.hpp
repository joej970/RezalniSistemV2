#ifndef SCREEN10SETORIGINVIEW_HPP
#define SCREEN10SETORIGINVIEW_HPP

#include <gui_generated/screen10setorigin_screen/Screen10SetOriginViewBase.hpp>
#include <gui/screen10setorigin_screen/Screen10SetOriginPresenter.hpp>

class Screen10SetOriginView : public Screen10SetOriginViewBase
{
public:
    Screen10SetOriginView();
    virtual ~Screen10SetOriginView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    void showOnConsole(){
    	popUpUartConsoleGRBL.setLinesText(presenter->fetchUartLineBuffers());
    	showUartConsole();
    }


    void requestUpdateOrigin(){
      	uint16_t x_01mm = digitSelector_4bits_x0.getCurrentValue();
        uint16_t y_01mm = digitSelector_4bits_y0.getCurrentValue();
        presenter->updateOrigin(x_01mm, y_01mm);
    }

    void updateOrigin(uint16_t x_01mm, uint16_t y_01mm){
    	presenter->updateOrigin(x_01mm, y_01mm);
    }

    void moveGRBLtoOrigin()
    {
    	uint16_t x_01mm = digitSelector_4bits_x0.getCurrentValue();
    	uint16_t y_01mm = digitSelector_4bits_y0.getCurrentValue();
        presenter->moveGRBL(x_01mm, y_01mm);
    }

    void moveGRBLtoHome()
    {
        presenter->moveGRBLtoHome();
    }

protected:

    void handleTickEvent();

    void hideUartConsole() {
    	popUpUartConsoleGRBL.hideUart();
    }

    void showUartConsole() {
    	popUpUartConsoleGRBL.setVisible(true);
    	popUpUartConsoleGRBL.invalidate();
    }

protected:
};

#endif // SCREEN10SETORIGINVIEW_HPP
