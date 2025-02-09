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
protected:
};

#endif // SCREEN10SETORIGINVIEW_HPP
