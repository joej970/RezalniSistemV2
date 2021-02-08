#include <gui/containers/SwipeDetectContainer.hpp>

SwipeDetectContainer::SwipeDetectContainer():
	SwipeDetectContainerBase(), action()
{
	setTouchable(true);
	setVisible(true); //handleGestureEvent is reported by HAL only to drawables that are visible and touchable only
}

void SwipeDetectContainer::initialize()
{
    SwipeDetectContainerBase::initialize();
}

void SwipeDetectContainer::handleGestureEvent(const GestureEvent& event){
	if(event.getType() == touchgfx::GestureEvent::SWIPE_HORIZONTAL){
		if(action -> isValid()){
			action->execute(event.getVelocity()); // <-- getVelocity returns int16_t
		}
	}
}
