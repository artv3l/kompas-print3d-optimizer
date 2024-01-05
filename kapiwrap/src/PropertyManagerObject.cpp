#include "PropertyManagerObject.hpp"

#include "PropertyManagerEvent.hpp"

PropertyManagerObject::PropertyManagerObject(KompasObjectPtr kompas) :
        m_kompas(kompas), m_application(kompas->ksGetApplication7()),
        m_propertyManager(m_application->CreatePropertyManager(true)),
        m_event(this) {
}

IPropertyManagerPtr PropertyManagerObject::getPropertyManager() {
    return m_propertyManager;
}

void PropertyManagerObject::show() {
    m_propertyManager->ShowTabs();
}

void PropertyManagerObject::hide() {
    m_propertyManager->HideTabs();
}
