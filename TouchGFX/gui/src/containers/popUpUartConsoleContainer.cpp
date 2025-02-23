#include <gui/containers/popUpUartConsoleContainer.hpp>
//#include <gui/model/Model.hpp>

popUpUartConsoleContainer::popUpUartConsoleContainer()
{

}


void popUpUartConsoleContainer::hideUart(){
	this->setVisible(false);
	this->invalidate();
//	if(m_model != NULL){
//		m_model->clearConsoleBuffers();
//	}
}
void popUpUartConsoleContainer::clearConsoleBuffers(){
	if(m_model != NULL){
		m_model->clearConsoleBuffers();
		setLinesText(m_model->fetchUartLineBuffers());
	}
//	this->invalidate();
}


void popUpUartConsoleContainer::setLinesText(std::vector<const char*> lineSources ){


//	const char* msg_0, const char* msg_1, const char* msg_2, const char* msg_3, const char* msg_4, const char* msg_5;
//	auto lines[] = {&uartConsole_line_0, &uartConsole_line_2, &uartConsole_line_3, &uartConsole_line_4, &uartConsole_line_5};
//	auto buffers[] = {}

	std::vector<touchgfx::TextAreaWithOneWildcard*> pLines{
			&uartConsole_line_0,
			&uartConsole_line_1,
			&uartConsole_line_2,
			&uartConsole_line_3,
			&uartConsole_line_4,
			&uartConsole_line_5,
			&uartConsole_line_6,
			&uartConsole_line_7,
			&uartConsole_line_8,
			&uartConsole_line_9
	};
	std::vector<touchgfx::Unicode::UnicodeChar*> pLineBuffers{
			uartConsole_line_0Buffer,
			uartConsole_line_1Buffer,
			uartConsole_line_2Buffer,
			uartConsole_line_3Buffer,
			uartConsole_line_4Buffer,
			uartConsole_line_5Buffer,
			uartConsole_line_6Buffer,
			uartConsole_line_7Buffer,
			uartConsole_line_8Buffer,
			uartConsole_line_9Buffer
	};

	for(uint16_t i = 0; i < lineSources.size(); i++ ){
		printf("console[%u]: message: %s (%u)\n", i, lineSources[i], Unicode::strlen(lineSources[i]));
		Unicode::strncpy(pLineBuffers[i], lineSources[i], Unicode::strlen(lineSources[i]));
		pLineBuffers[i][Unicode::strlen(lineSources[i])] = '\0';
		pLines[i]->setWideTextAction(WIDE_TEXT_NONE);
		pLines[i]->invalidate();
	}


//	uartConsole_line_0.setWideTextAction(WIDE_TEXT_NONE);
//	uartConsole_line_1.setWideTextAction(WIDE_TEXT_NONE);
//	uartConsole_line_2.setWideTextAction(WIDE_TEXT_NONE);
//	uartConsole_line_3.setWideTextAction(WIDE_TEXT_NONE);
//	uartConsole_line_4.setWideTextAction(WIDE_TEXT_NONE);
//	uartConsole_line_5.setWideTextAction(WIDE_TEXT_NONE);


//	Unicode::strncpy(uartConsole_line_1Buffer, lineSources[1], Unicode::strlen(lineSources[1]));
//	uartConsole_line_1Buffer[Unicode::strlen(lineSources[1])] = '\0';
//	Unicode::strncpy(uartConsole_line_2Buffer, lineSources[2], Unicode::strlen(lineSources[2]));
//	uartConsole_line_2Buffer[Unicode::strlen(lineSources[2])] = '\0';
//	Unicode::strncpy(uartConsole_line_3Buffer, lineSources[3], Unicode::strlen(lineSources[3]));
//	uartConsole_line_3Buffer[Unicode::strlen(lineSources[3])] = '\0';
//	Unicode::strncpy(uartConsole_line_4Buffer, lineSources[4], Unicode::strlen(lineSources[4]));
//	uartConsole_line_4Buffer[Unicode::strlen(lineSources[4])] = '\0';
//	Unicode::strncpy(uartConsole_line_5Buffer, lineSources[5], Unicode::strlen(lineSources[5]));
//	uartConsole_line_5Buffer[Unicode::strlen(lineSources[5])] = '\0';

//	uartConsole_line_1.invalidate();
//	uartConsole_line_2.invalidate();
//	uartConsole_line_3.invalidate();
//	uartConsole_line_4.invalidate();
//	uartConsole_line_5.invalidate();
}


void popUpUartConsoleContainer::initialize()
{
    popUpUartConsoleContainerBase::initialize();
}
