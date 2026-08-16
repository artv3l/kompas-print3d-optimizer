#include "kapiwrap/PropertyManagerObject.hpp"

#include "kapiwrap/PropertyManagerEvent.hpp"

PropertyManagerObject::PropertyManagerObject(kapi::KompasObjectPtr kompas) :
        m_kompas(kompas), m_application(kompas->ksGetApplication7()),
        m_propertyManager(m_application->CreatePropertyManager(true)),
        m_event(this) {
}

kapi::IPropertyManagerPtr PropertyManagerObject::getPropertyManager() {
    return m_propertyManager;
}

bool PropertyManagerObject::buttonClick(long buttonId) {
    return false;
}

bool PropertyManagerObject::changeControlValue(IDispatch* control) {
    return false;
}

bool PropertyManagerObject::controlCommand(IDispatch* control, long buttonId) {
    return false;
}

bool PropertyManagerObject::buttonUpdate(long buttonId, long* check, short* enable) {
    return false;
}

bool PropertyManagerObject::processActivate() {
    return false;
}

bool PropertyManagerObject::processDeactivate() {
    return false;
}

bool PropertyManagerObject::commandHelp(long buttonId) {
    return false;
}

bool PropertyManagerObject::selectItem(IDispatch* control, long index, bool select) {
    return false;
}

void PropertyManagerObject::show() {
    m_propertyManager->ShowTabs();
}

void PropertyManagerObject::hide() {
    m_propertyManager->HideTabs();
}
