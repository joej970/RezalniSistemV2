#ifndef MODEL_HPP
#define MODEL_HPP

#include <touchgfx/hal/types.hpp>

class ModelListener;

class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();


    void setRelay1duration(uint32_t duration);
    void setRelay1delay(uint32_t delay);
    void setRelay2duration(uint32_t duration);
    void setRelay2delay(uint32_t delay);
    void setRelay3duration(uint32_t duration);
    void setRelay3delay(uint32_t delay);

    uint32_t getRelay1duration();
    uint32_t getRelay1delay();
    uint32_t getRelay2duration();
    uint32_t getRelay2delay();
    uint32_t getRelay3duration();
    uint32_t getRelay3delay();



protected:
    ModelListener* modelListener;
    uint32_t relay1duration;
    uint32_t relay1delay;
    uint32_t relay2duration;
    uint32_t relay2delay;
    uint32_t relay3duration;
    uint32_t relay3delay;
};

#endif // MODEL_HPP
