#include "Macro.hpp"

Macro::Macro(kapi::ksPartPtr part, _bstr_t name, bool staffVisible):
        m_entity(part->NewEntity(kapi::o3d_MacroObject)),
        m_definition(m_entity->GetDefinition()) {
    m_entity->name = name;
    m_definition->StaffVisible = staffVisible;
    m_entity->Create();
}

Macro::Macro(kapi::ksEntityPtr entity) :
    m_entity(entity), m_definition(m_entity->GetDefinition())
{}


Macro::~Macro() {
    update();
}

kapi::ksEntityPtr Macro::findMacro(kapi::ksPartPtr part, _bstr_t name) {
    kapi::ksEntityCollectionPtr macroCollection = part->EntityCollection(kapi::Obj3dType::o3d_MacroObject);
    return macroCollection->GetByName(name, true, false);
}

bool Macro::add(IDispatchPtr obj) {
    return m_definition->Add(obj);
}

bool Macro::add(Macro macro) {
    return m_definition->Add(macro.m_entity);
}

bool Macro::update() {
    return m_entity->Update();
}

bool Macro::isCreated() const {
    return m_entity->IsCreated();
}

_bstr_t Macro::getName() const {
    return m_entity->name;
}

kapi::ksEntityPtr Macro::getEntity() const {
    return m_entity;
}
