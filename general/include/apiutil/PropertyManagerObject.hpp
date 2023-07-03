#ifndef PROPERTY_MANAGER_OBJECT_HPP
#define PROPERTY_MANAGER_OBJECT_HPP

#include "stdafx.h"

class PropertyManagerEvent;

class PropertyManagerObject {
public:
    PropertyManagerObject(KompasObjectPtr kompas);

    IPropertyManagerPtr getPropertyManager();

    virtual bool buttonClick(long buttonId) = 0;

    virtual void show();
    virtual void hide();

protected:
    KompasObjectPtr m_kompas;
    IApplicationPtr m_application;
    IPropertyManagerPtr m_propertyManager;

private:
    PropertyManagerEvent *m_event;

};

#endif /* PROPERTY_MANAGER_OBJECT_HPP */
