#ifndef POPUPWINDOW_HPP
#define POPUPWINDOW_HPP

#include <gui_generated/containers/popUpWindowBase.hpp>
#include <texts/TextKeysAndLanguages.hpp>


class popUpWindow : public popUpWindowBase
{
public:
    popUpWindow();
    virtual ~popUpWindow() {}

    void hidePopUp();
    void setText(TEXTS textId);
    void setTextWithMessage(TEXTS textId, const char* message);

    virtual void initialize();
protected:
};

#endif // POPUPWINDOW_HPP
