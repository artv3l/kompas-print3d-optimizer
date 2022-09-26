#include "stdafx.h"
#include "Macro.hpp"

Macro::Macro(ksPartPtr part, _bstr_t name, bool staffVisible):
        entity_(part->NewEntity(o3d_MacroObject)),
        definition_(entity_->GetDefinition()) {
    entity_->name = name;
    definition_->StaffVisible = staffVisible;
    entity_->Create();
}

Macro::~Macro() {
    update();
}

bool Macro::add(IDispatchPtr obj) {
    return definition_->Add(obj);
}

bool Macro::add(Macro macro) {
    return definition_->Add(macro.entity_);
}

bool Macro::update() {
    return entity_->Update();
}
