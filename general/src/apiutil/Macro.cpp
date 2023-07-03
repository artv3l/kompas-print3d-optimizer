#include "stdafx.h"
#include "apiutil/Macro.hpp"

Macro::Macro(ksPartPtr part, _bstr_t name, bool staffVisible):
        entity_(part->NewEntity(o3d_MacroObject)),
        definition_(entity_->GetDefinition()) {
    entity_->name = name;
    definition_->StaffVisible = staffVisible;
    entity_->Create();
}

Macro::Macro(ksEntityPtr entity) :
    entity_(entity), definition_(entity_->GetDefinition())
{}


Macro::~Macro() {
    update();
}

ksEntityPtr Macro::findMacro(ksPartPtr part, _bstr_t name) {
    ksEntityCollectionPtr macroCollection = part->EntityCollection(Obj3dType::o3d_MacroObject);
    return macroCollection->GetByName(name, true, false);
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

bool Macro::isCreated() const {
    return entity_->IsCreated();
}

_bstr_t Macro::getName() const {
    return entity_->name;
}
