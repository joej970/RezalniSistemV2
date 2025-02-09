#include <gui/containers/popUpUartConsoleContainer.hpp>

popUpUartConsoleContainer::popUpUartConsoleContainer()
{

}


void popUpUartConsoleContainer::hideUart(){
	this->setVisible(false);
	this->invalidate();
//	statusText.invalidate();
}


//void setLinesText(const char* msg_0, const char* msg_1, const char* msg_2, const char* msg_3, const char* msg_4, const char* msg_5){

void popUpUartConsoleContainer::setLinesText(std::vector<const char*> lineSources ){


//	const char* msg_0, const char* msg_1, const char* msg_2, const char* msg_3, const char* msg_4, const char* msg_5;
//	auto lines[] = {&uartConsole_line_0, &uartConsole_line_2, &uartConsole_line_3, &uartConsole_line_4, &uartConsole_line_5};
//	auto buffers[] = {}

	uartConsole_line_0.setWideTextAction(WIDE_TEXT_NONE);
	uartConsole_line_1.setWideTextAction(WIDE_TEXT_NONE);
	uartConsole_line_2.setWideTextAction(WIDE_TEXT_NONE);
	uartConsole_line_3.setWideTextAction(WIDE_TEXT_NONE);
	uartConsole_line_4.setWideTextAction(WIDE_TEXT_NONE);
	uartConsole_line_5.setWideTextAction(WIDE_TEXT_NONE);

	Unicode::strncpy(uartConsole_line_0Buffer, lineSources[0], Unicode::strlen(lineSources[0]));
	Unicode::strncpy(uartConsole_line_1Buffer, lineSources[1], Unicode::strlen(lineSources[1]));
	Unicode::strncpy(uartConsole_line_2Buffer, lineSources[2], Unicode::strlen(lineSources[2]));
	Unicode::strncpy(uartConsole_line_3Buffer, lineSources[3], Unicode::strlen(lineSources[3]));
	Unicode::strncpy(uartConsole_line_4Buffer, lineSources[4], Unicode::strlen(lineSources[4]));
	Unicode::strncpy(uartConsole_line_5Buffer, lineSources[5], Unicode::strlen(lineSources[5]));

	uartConsole_line_0.invalidate();
	uartConsole_line_1.invalidate();
	uartConsole_line_2.invalidate();
	uartConsole_line_3.invalidate();
	uartConsole_line_4.invalidate();
	uartConsole_line_5.invalidate();
}


void popUpUartConsoleContainer::initialize()
{
    popUpUartConsoleContainerBase::initialize();
}
