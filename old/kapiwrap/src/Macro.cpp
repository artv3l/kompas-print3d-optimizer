#include "kapiwrap/Macro.hpp"

Macro::Macro(kapi::ksPartPtr part, _bstr_t name, bool staffVisible):
        m_entity(part->NewEntity(kapi::o3d_MacroObject)),
        m_definition(m_entity->GetDefinition()) {
    m_entity->name = name;
    m_definition->StaffVisible = staffVisible;
    m_entity->Create();
}

Macro::Macro(ksapi::IPartPtr part, std::wstring_view name, bool staffVisible)
{
    auto macros = part->GetModelContainer()->GetMacroObjects3D();
    m_macro3d = macros->Add();
    m_macro3d->SetName(name.data());
    m_macro3d->SetStaffVisible(staffVisible);
}

Macro::Macro(kapi::ksEntityPtr entity) :
    m_entity(entity), m_definition(m_entity->GetDefinition())
{}


Macro::~Macro()
{
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
    if (m_definition)
        m_definition->Add(macro.m_entity);
    else
        add(macro.getModelObject());
    return true;
}

void Macro::add(ksapi::IModelObjectPtr object)
{
    m_objects.push_back(object);
}

bool Macro::update()
{
    bool result = false;
    if (m_entity)
        result = m_entity->Update();
    if (m_macro3d) {
        m_macro3d->SetObjects(m_objects);
        m_macro3d->Update();
    }
    return result;
}

bool Macro::isCreated() const {
    return m_entity->IsCreated();
}

_bstr_t Macro::getName() const {
    return m_entity->name;
}

kapi::ksEntityPtr Macro::getEntity() const
{
    return m_entity;
}

ksapi::IModelObjectPtr Macro::getModelObject() const
{
    return m_macro3d;
}
