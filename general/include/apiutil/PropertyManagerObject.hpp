#ifndef PROPERTY_MANAGER_OBJECT_HPP
#define PROPERTY_MANAGER_OBJECT_HPP

#include "stdafx.h"

class PropertyManagerEvent;

class PropertyManagerObject {
public:
    PropertyManagerObject(KompasObjectPtr kompas);

    IPropertyManagerPtr getPropertyManager();

    virtual bool buttonClick(long buttonId) = 0;

    void show();
    void hide();

protected:
    KompasObjectPtr kompas_;
    IApplicationPtr application_;
    IPropertyManagerPtr propertyManager_;

private:
    PropertyManagerEvent *event_;

};

#endif /* PROPERTY_MANAGER_OBJECT_HPP */
