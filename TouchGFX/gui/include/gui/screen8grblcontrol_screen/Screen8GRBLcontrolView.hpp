#ifndef SCREEN8GRBLCONTROLVIEW_HPP
#define SCREEN8GRBLCONTROLVIEW_HPP

#include <gui_generated/screen8grblcontrol_screen/Screen8GRBLcontrolViewBase.hpp>
#include <gui/screen8grblcontrol_screen/Screen8GRBLcontrolPresenter.hpp>
#include <gui/model/Model.hpp>

class Screen8GRBLcontrolView : public Screen8GRBLcontrolViewBase
{
public:
    Screen8GRBLcontrolView();
    virtual ~Screen8GRBLcontrolView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    void showHomingConfirmPrompt(){
 	   grblHomingRequest.openHomingPrompt();
    }

    void showOnConsole(){
    	popUpUartConsoleGRBL.setLinesText(presenter->fetchUartLineBuffers());
    	showUartConsole();
    }

    void updateProductionRate(double velocity){
    	Unicode::snprintfFloat(textAreaProductionVelocityBuffer, TEXTAREAPRODUCTIONVELOCITY_SIZE, "%0.1f", velocity);
    	textAreaProductionVelocity.invalidate();
    }

protected:

    touchgfx::Callback<Screen8GRBLcontrolView, int16_t> swipeCallback;

    void swipeCallbackHandler(int16_t);

    void handleTickEvent();

    void hideUartConsole() {
    	popUpUartConsoleGRBL.hideUart();
    }

    void showUartConsole() {
    	popUpUartConsoleGRBL.setVisible(true);
    	popUpUartConsoleGRBL.invalidate();
    }

    void tryToConnectGRBL()
	{
		presenter->tryToConnectGRBL();
	}





//    Model& getModelFromPresenter(){
//    	return presenter->getModel();
//    }

//    enum grblConn_t{
//    	NOT_CONNECTED,
//    	PENDING,
//    	CONNECTED
//    };
//
//    enum grblConn_t lastGRBLStatus_temp;

};

#endif // SCREEN8GRBLCONTROLVIEW_HPP
