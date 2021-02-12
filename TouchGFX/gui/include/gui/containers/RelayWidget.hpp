#ifndef RELAYWIDGET_HPP
#define RELAYWIDGET_HPP

#include <gui_generated/containers/RelayWidgetBase.hpp>

class RelayWidget : public RelayWidgetBase
{
public:
    RelayWidget();
    virtual ~RelayWidget() {}

    virtual void initialize();

    void setId(int8_t id){
    	Unicode::snprintf(textRelayIDBuffer, TEXTRELAYID_SIZE, "%d", id);
    	textRelayID.invalidate();
    }

    void setDelay(uint32_t delay){
    	Unicode::snprintf(textDelayValueBuffer, TEXTDELAYVALUE_SIZE, "%u", delay);
    	textDelayValue.invalidate();
    }

    void setDuration(uint32_t duration){
    	Unicode::snprintf(textDurationValueBuffer, TEXTDURATIONVALUE_SIZE, "%u", duration);
    	textDurationValue.invalidate();
    }

protected:
};

#endif // RELAYWIDGET_HPP
