#ifndef PROPERTY_MANAGER_OBJECT_HPP
#define PROPERTY_MANAGER_OBJECT_HPP

#include "PropertyManagerEvent.hpp"

class PropertyManagerObject {
public:
    PropertyManagerObject(kapi::KompasObjectPtr kompas);

    kapi::IPropertyManagerPtr getPropertyManager();

    virtual bool buttonClick(long buttonId) = 0;

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
