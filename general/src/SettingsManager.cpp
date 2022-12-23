#include "stdafx.h"
#include "SettingsManager.hpp"

#include "PropertyManagerObject.hpp"

SettingsManager::SettingsManager(KompasObjectPtr kompas):
        PropertyManagerObject(kompas) {
    propertyManager_->Layout = PropertyManagerLayout::pmAlignRight;
    propertyManager_->SpecToolbar = SpecPropertyToolBarEnum::pnEnterEscHelp;
    propertyManager_->Caption = _T("Настройки");

    IPropertyTabPtr tab(propertyManager_->PropertyTabs->Add(_T("Закладка из библиотеки")));

    IPropertyControlsPtr collection = tab->PropertyControls;
    IPropertyEditPtr edit = collection->Add(ksControlEditInt);
    edit->Name = _T("IPropertyEdit");
    edit->Id = 100100;
}

bool SettingsManager::buttonClick(long buttonId) {
    switch (buttonId) {
    case SpecPropertyButtonEnum::pbEnter:
        // todo
        kompas_->ksMessage("enter");
        break;
    case SpecPropertyButtonEnum::pbHelp:
        // todo
        kompas_->ksMessage("help");
        break;
    case SpecPropertyButtonEnum::pbEsc:
        hide();
        break;
    }
    return true;
}
