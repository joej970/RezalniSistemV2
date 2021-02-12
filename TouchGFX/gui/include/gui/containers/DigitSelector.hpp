#ifndef DIGITSELECTOR_HPP
#define DIGITSELECTOR_HPP

#include <gui_generated/containers/DigitSelectorBase.hpp>

class DigitSelector : public DigitSelectorBase
{
public:
    DigitSelector();
    virtual ~DigitSelector() {}

    virtual void initialize();

    virtual void scrollWheelDigitE_0UpdateItem(DigitElement& item, int16_t itemIndex)
    {
        item.setNumber(itemIndex);
    }

    virtual void scrollWheelDigitE_1UpdateItem(DigitElement& item, int16_t itemIndex)
    {
    	item.setNumber(itemIndex);
    }

    virtual void scrollWheelDigitE_2UpdateItem(DigitElement& item, int16_t itemIndex)
    {
    	item.setNumber(itemIndex);
    }

    virtual void scrollWheelDigitE_3UpdateItem(DigitElement& item, int16_t itemIndex)
    {
    	item.setNumber(itemIndex);
    }

    virtual void scrollWheelDigitE_4UpdateItem(DigitElement& item, int16_t itemIndex)
    {
    	item.setNumber(itemIndex);
    }

    virtual void scrollWheelDigitE_5UpdateItem(DigitElement& item, int16_t itemIndex)
    {
    	item.setNumber(itemIndex);
    }

    virtual void setCurrentValue(uint32_t currentValue){
    	scrollWheelDigitE_0.animateToItem(currentValue/1 %10, 0);
    	scrollWheelDigitE_1.animateToItem(currentValue/10 %10, 0);
    	scrollWheelDigitE_2.animateToItem(currentValue/100 %10, 0);
    	scrollWheelDigitE_3.animateToItem(currentValue/1000 %10, 0);
    	scrollWheelDigitE_4.animateToItem(currentValue/10000 %10, 0);
    	scrollWheelDigitE_5.animateToItem(currentValue/100000 %10, 0);

    }

    virtual uint32_t getCurrentValue(){
    	uint32_t currentValue = scrollWheelDigitE_0.getSelectedItem();
    	currentValue += 10*scrollWheelDigitE_1.getSelectedItem();
    	currentValue += 100*scrollWheelDigitE_2.getSelectedItem();
    	currentValue += 1000*scrollWheelDigitE_3.getSelectedItem();
    	currentValue += 10000*scrollWheelDigitE_4.getSelectedItem();
    	currentValue += 100000*scrollWheelDigitE_5.getSelectedItem();
    	return currentValue;
    }
protected:
};

#endif // DIGITSELECTOR_HPP
