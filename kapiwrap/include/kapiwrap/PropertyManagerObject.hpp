#ifndef PROPERTY_MANAGER_OBJECT_HPP
#define PROPERTY_MANAGER_OBJECT_HPP

#include "PropertyManagerEvent.hpp"

class PropertyManagerObject {
public:
    PropertyManagerObject(kapi::KompasObjectPtr kompas);
    virtual ~PropertyManagerObject() = default;

    kapi::IPropertyManagerPtr getPropertyManager();

    virtual bool buttonClick(long buttonId);
    virtual bool changeControlValue(IDispatch* control);
    virtual bool controlCommand(IDispatch* control, long buttonId);
    virtual bool buttonUpdate(long buttonId, long* check, short* enable);
    virtual bool processActivate();
    virtual bool processDeactivate();
    virtual bool commandHelp(long buttonId);

    virtual void show();
    virtual void hide();

protected:
    kapi::KompasObjectPtr m_kompas;
    kapi::IApplicationPtr m_application;
    kapi::IPropertyManagerPtr m_propertyManager;

private:
    PropertyManagerEvent m_event;

};

#endif /* PROPERTY_MANAGER_OBJECT_HPP */
