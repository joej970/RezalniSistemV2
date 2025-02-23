#include <gui/screen8grblcontrol_screen/Screen8GRBLcontrolView.hpp>
#include <gui/screen8grblcontrol_screen/Screen8GRBLcontrolPresenter.hpp>

Screen8GRBLcontrolPresenter::Screen8GRBLcontrolPresenter(Screen8GRBLcontrolView& v)
    : view(v)
{

}

void Screen8GRBLcontrolPresenter::activate()
{

}

void Screen8GRBLcontrolPresenter::deactivate()
{

}

void Screen8GRBLcontrolPresenter::onConsoleDataUpdated(){
	view.showOnConsole();
};

//void Screen8GRBLcontrolPresenter::onConsoleDataUpdated(){
//	view.popUpUartConsoleGRBL.setVisible();
//};


//bool Screen8GRBLcontrolPresenter::shouldUartConsoleBeVisible(){
////	return model->shouldUartConsoleBeVisible();
//}
