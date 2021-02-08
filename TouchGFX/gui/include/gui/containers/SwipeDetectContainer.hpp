#ifndef SWIPEDETECTCONTAINER_HPP
#define SWIPEDETECTCONTAINER_HPP

#include <gui_generated/containers/SwipeDetectContainerBase.hpp>
#include <touchgfx/Callback.hpp>

class SwipeDetectContainer : public SwipeDetectContainerBase
{
public:
    SwipeDetectContainer();
    virtual ~SwipeDetectContainer() {}

    virtual void initialize();

    virtual void setAction(GenericCallback<int16_t> &callback){
          action = &callback;
        }

    virtual void handleGestureEvent(const GestureEvent& event);
protected:
    GenericCallback<int16_t>* action;

};

#endif // SWIPEDETECTCONTAINER_HPP
