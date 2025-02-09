#ifndef DIGITSELECTOR_4BITS_HPP
#define DIGITSELECTOR_4BITS_HPP

#include <gui_generated/containers/DigitSelector_4bitsBase.hpp>

class DigitSelector_4bits : public DigitSelector_4bitsBase
{
public:
    DigitSelector_4bits();
    virtual ~DigitSelector_4bits() {}

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


        virtual void setCurrentValue(uint16_t currentValue){
        	scrollWheelDigitE_0.animateToItem(currentValue/1 %10, 0);
        	scrollWheelDigitE_1.animateToItem(currentValue/10 %10, 0);
        	scrollWheelDigitE_2.animateToItem(currentValue/100 %10, 0);
        	scrollWheelDigitE_3.animateToItem(currentValue/1000 %10, 0);
		}

        virtual uint16_t getCurrentValue(){
        	uint16_t currentValue = scrollWheelDigitE_0.getSelectedItem();
        	currentValue += 10*scrollWheelDigitE_1.getSelectedItem();
        	currentValue += 100*scrollWheelDigitE_2.getSelectedItem();
        	currentValue += 1000*scrollWheelDigitE_3.getSelectedItem();
        	return currentValue;
        }
protected:
};

#endif // DIGITSELECTOR_4BITS_HPP
