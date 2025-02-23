#ifndef POPUPUARTCONSOLECONTAINER_HPP
#define POPUPUARTCONSOLECONTAINER_HPP

#include <gui_generated/containers/popUpUartConsoleContainerBase.hpp>
#include <vector>
#include <gui/model/Model.hpp>

class popUpUartConsoleContainer : public popUpUartConsoleContainerBase
{
public:
    popUpUartConsoleContainer();
    virtual ~popUpUartConsoleContainer() {}

    void hideUart();
    void clearConsoleBuffers();

//    void setText(const char* message);
//    void setLinesText(const char* msg_0, const char* msg_1, const char* msg_2, const char* msg_3, const char* msg_4, const char* msg_5);
    void setLinesText(std::vector<const char*> lineSources );

    virtual void initialize();
    void setModel(Model* model){
    	m_model = model;
    }

protected:

    Model* m_model = NULL;

};

#endif // POPUPUARTCONSOLECONTAINER_HPP
