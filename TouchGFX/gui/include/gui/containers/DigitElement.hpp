#ifndef DIGITELEMENT_HPP
#define DIGITELEMENT_HPP

#include <gui_generated/containers/DigitElementBase.hpp>

class DigitElement : public DigitElementBase
{
public:
    DigitElement();
    virtual ~DigitElement() {}

    virtual void initialize();

    virtual void setNumber(int16_t itemIndex){
    	Unicode::itoa(itemIndex, textDigitNrBuffer, TEXTDIGITNR_SIZE, 10);
    }
protected:
};

#endif // DIGITELEMENT_HPP
