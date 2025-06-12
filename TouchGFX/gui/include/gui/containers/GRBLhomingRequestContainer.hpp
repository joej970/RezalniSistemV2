#ifndef GRBLHOMINGREQUESTCONTAINER_HPP
#define GRBLHOMINGREQUESTCONTAINER_HPP

#include <gui_generated/containers/GRBLhomingRequestContainerBase.hpp>

class GRBLhomingRequestContainer : public GRBLhomingRequestContainerBase
{
public:
    GRBLhomingRequestContainer();
    virtual ~GRBLhomingRequestContainer() {}

    virtual void initialize();

    void setModel(Model* model){
    	m_model = model;
    }

    void openHomingPrompt(){
    	this->setVisible(true);
    	this->invalidate();
    }

    void hideHomingPrompt(){
    	this->setVisible(false);
    	this->invalidate();
    }

    void tellHomingConfirmed()
    {
    	hideHomingPrompt();
        m_model->homingConfirmed(true);
    }

    void tellHomingDenied()
    {
    	hideHomingPrompt();
        m_model->homingConfirmed(false);
    }

protected:

    Model* m_model = NULL;
};

#endif // GRBLHOMINGREQUESTCONTAINER_HPP
