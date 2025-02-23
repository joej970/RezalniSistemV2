#include <gui/screen1maincutting_screen/Screen1MainCuttingView.hpp>
#include <gui/screen1maincutting_screen/Screen1MainCuttingPresenter.hpp>

Screen1MainCuttingPresenter::Screen1MainCuttingPresenter(Screen1MainCuttingView& v)
    : view(v)
{

}

void Screen1MainCuttingPresenter::activate()
{

}

void Screen1MainCuttingPresenter::deactivate()
{

}

void Screen1MainCuttingPresenter::onConsoleDataUpdated(){
	view.showOnConsole();
};
