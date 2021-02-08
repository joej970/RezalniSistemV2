#ifndef RELAYWIDGET_HPP
#define RELAYWIDGET_HPP

#include <gui_generated/containers/RelayWidgetBase.hpp>

class RelayWidget : public RelayWidgetBase
{
public:
    RelayWidget();
    virtual ~RelayWidget() {}

    virtual void initialize();
protected:
};

#endif // RELAYWIDGET_HPP
