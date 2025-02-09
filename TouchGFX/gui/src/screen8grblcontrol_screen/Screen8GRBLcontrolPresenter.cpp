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


std::vector<const char*> Screen8GRBLcontrolPresenter::fetchUartLineBuffers(){
	return model->fetchUartLineBuffers();
}

//bool Screen8GRBLcontrolPresenter::shouldUartConsoleBeVisible(){
////	return model->shouldUartConsoleBeVisible();
//}
