#include "stdafx.h"
#include "PropertyManagerObject.hpp"

#include "PropertyManagerEvent.hpp"

PropertyManagerObject::PropertyManagerObject(KompasObjectPtr kompas) :
        kompas_(kompas), application_(kompas->ksGetApplication7()),
        propertyManager_(application_->CreatePropertyManager(true)),
        event_(new PropertyManagerEvent(this)) {
}

IPropertyManagerPtr PropertyManagerObject::getPropertyManager() {
    return propertyManager_;
}

void PropertyManagerObject::show() {
    propertyManager_->ShowTabs();
}

void PropertyManagerObject::hide() {
    propertyManager_->HideTabs();
}
