#ifndef MODEL_HPP
#define MODEL_HPP

#include <touchgfx/hal/types.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Texts.hpp>
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
	void setRadius(uint16_t radius);
    void resetAmount(void);
    void resetCurrLength(void);
    void enableCutting(bool enable);
    void immCut();
	void updateSetLength(uint32_t newLength);
	void saveRelaySettings(uint32_t id);
	void saveEncoderSettings();
	void loadSettings(void);
	void reportToEncoderControlTask();
	void reportToRelaySetupTask(uint32_t id);
	void resetLastStatus();
	void toggleRelaysActive();
	void saveLanguage(LANGUAGES language);


    uint32_t getRelay1duration();
    uint32_t getRelay1delay();
    uint32_t getRelay2duration();
    uint32_t getRelay2delay();
    uint32_t getRelay3duration();
    uint32_t getRelay3delay();
	uint16_t getRadius();
    uint32_t getAmount();
    bool getCutting();
	uint32_t getSetLength();
	uint32_t getSetLengthActual();
	uint32_t getCurrLength();
	statusId_t getLastStatus();
	uint32_t getStatusPackageData();
	char* getMessage();
	uint8_t getRelaysActive();


protected:
    ModelListener* modelListener;
    uint32_t relay1duration;
    uint32_t relay1delay;
    uint32_t relay2duration;
    uint32_t relay2delay;
    uint32_t relay3duration;
    uint32_t relay3delay;
    uint32_t amount;
    uint32_t setLength;
    uint32_t setLengthActual;
    uint32_t currLength;
    uint16_t radius_01mm;
    uint16_t resolution;
    bool cuttingActive;
    //bool immediateCut;
    statusId_t lastStatus;
    bool fetchSettings;
    uint32_t statusPackageData;
    char message[30];
    char* p_message;
    LANGUAGES languageIdx;
    uint8_t relaysActive;
    uint16_t brightness;


};

#endif // MODEL_HPP
