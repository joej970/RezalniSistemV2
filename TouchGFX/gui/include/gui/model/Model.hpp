#ifndef MODEL_HPP
#define MODEL_HPP

#include <touchgfx/hal/types.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Texts.hpp>
#include <vector>
//#include <deque>
#include <string>
//#include <gui_generated/containers/popUpUartConsoleContainerBase.hpp>
#include "main.h"
#include "qPackages.h"

extern grblConn_t grblConnectionStatus;

class ModelListener;

class Model {
public:
	Model();

	void bind(ModelListener *listener) {
		modelListener = listener;
	}

	void tick();

	// SETTERS. Writing to EEPROM.
	void setAlpha(uint16_t angle);
	void setBeta(uint16_t beta);
	void setFeedrate(uint16_t fr);
	void setWidth(uint16_t width);

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
	void writeLaserParamsToEEPROM(uint8_t slot);

	// REPORT
	void reportToEncoderControlTask();
	void reportToRelaySetupTask(uint32_t id);
	void reportToGRBLControlTask();
	void resetLastStatus();
	void toggleRelaysActive();
	void saveLanguage(LANGUAGES language);

	// GETTERS. Reading from EEPROM.
	void loadSettings(void);

	uint16_t getAlpha();
	uint16_t getBeta();
	uint16_t getFeedrate();
	uint16_t getWidth();

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
	void updateLaserParamsFromEEPROM(uint8_t slot);

	// Console functions
	std::vector<const char*> fetchUartLineBuffers();
	void clearConsoleBuffers();
	void processUARTData(qPackage_UART uartPayload);
	void tellConsoleVisible();

	// GRBL functions
	void tryToConnectGRBL();
	void sendGRBLtoHome();
	void sendGRBLtoOrigin();

	/* tell the caller whether it should re-fetch data from model*/
	bool shouldUpdateLaserParams(){
		bool state = laserParamsUpdatedFromEEPROM;
		if(state == true){
			laserParamsUpdatedFromEEPROM = false;
		}
		return state;
	}

	uint16_t getOrigin_X0(){return origin_x0_01mm;}
	uint16_t getOrigin_Y0(){return origin_y0_01mm;}
	void setOrigin_X0(uint16_t value){origin_x0_01mm = value;}
	void setOrigin_Y0(uint16_t value){origin_y0_01mm = value;}

    grblConn_t getGRBLconnStatus(){
    	return grblConnectionStatus;
    }


protected:

	ModelListener *modelListener;
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
	char *p_message;
	LANGUAGES languageIdx;
	uint8_t relaysActive;
	uint16_t brightness;

	uint16_t angle_alpha_01deg;
	uint16_t angle_beta_01deg;
	uint16_t feedrate;
	uint16_t width_01mm;
	uint16_t origin_y0_01mm;
	uint16_t origin_x0_01mm;
	bool laserParamsUpdatedFromEEPROM;




#define LINE_BUFFERS_SIZE (uint16_t) 100
#define NR_LINES (uint16_t) 10

	struct consoleBuffers_s {
		std::vector<std::string> lineBuffers { NR_LINES };
		bool allLinesFull = false;
		enum uart_msg_type_e type_last = RX;

		void resetConBuffers(){
			for(auto& lineBuffer : lineBuffers){
				lineBuffer.clear();
			}
			currBuffer = 0;
			allLinesFull = false;
			type_last = RX;
		}

		void incCurrBuffer(){
			currBuffer++;
			if(currBuffer >= NR_LINES){
				allLinesFull = true;
				currBuffer = 0;
			}

		}

		void decCurrBuffer(){
			currBuffer--;
			if(currBuffer < 0){
				currBuffer = NR_LINES - 1;
			}
		}

		int16_t getCurrBuffer(){
			return currBuffer;
		}
	private:
		int16_t currBuffer = 0; // will be overflown to 0 when called for the first time

	} conBufs;

//	bool uartShouldBeVisible;
};

//    	char line[100];// = '\0';
//    	char buffer[popUpUartConsoleContainerBase::UARTCONSOLE_SIZE] = '\0';
//    	const uint16_t capacity = popUpUartConsoleContainerBase::UARTCONSOLE_SIZE;

#endif // MODEL_HPP
