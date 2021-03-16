#ifndef MODEL_HPP
#define MODEL_HPP

#include <touchgfx/hal/types.hpp>
//#include "status_enum.h"
#include "main.h"


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
    void resetAmmount(void);
    void enableCutting(bool enable);
    void immCut();
	void updateSetLength(uint32_t newLength);
	void reportToEncoderControlTask();
	void reportToRelaySetupTask(uint32_t id);
	void resetLastStatus();

    uint32_t getRelay1duration();
    uint32_t getRelay1delay();
    uint32_t getRelay2duration();
    uint32_t getRelay2delay();
    uint32_t getRelay3duration();
    uint32_t getRelay3delay();
    uint32_t getAmmount();
    bool getCutting();
	uint32_t getSetLength();
	uint32_t getSetLengthActual();
	uint32_t getCurrLength();
	statusId_t getLastStatus();


protected:
    ModelListener* modelListener;
    uint32_t relay1duration;
    uint32_t relay1delay;
    uint32_t relay2duration;
    uint32_t relay2delay;
    uint32_t relay3duration;
    uint32_t relay3delay;
    uint32_t ammount;
    uint32_t setLength;
    uint32_t setLengthActual;
    uint32_t currLength;
    bool cuttingActive;
    bool immediateCut;
    statusId_t lastStatus;



};

#endif // MODEL_HPP
