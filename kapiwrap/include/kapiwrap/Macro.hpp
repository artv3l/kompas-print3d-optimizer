#ifndef MACRO_HPP
#define MACRO_HPP

#include <string>

#include <comutil.h>

#include <KsAPI.h>

class Macro {
public:
    Macro(kapi::ksPartPtr part, _bstr_t name, bool staffVisible);
    Macro(ksapi::IPartPtr part, std::wstring_view name, bool staffVisible);
    explicit Macro(kapi::ksEntityPtr entity);

    ~Macro();
    
    static kapi::ksEntityPtr findMacro(kapi::ksPartPtr part, _bstr_t name);

    bool add(IDispatchPtr obj);
    bool add(Macro macro);
    void add(ksapi::IModelObjectPtr object);
    bool update();
    bool isCreated() const;
    _bstr_t getName() const;
    kapi::ksEntityPtr getEntity() const;
    ksapi::IModelObjectPtr getModelObject() const;

private:
    kapi::ksEntityPtr m_entity;
    kapi::ksMacro3DDefinitionPtr m_definition;

    ksapi::IMacroObject3DPtr m_macro3d;
    std::vector<ksapi::IModelObjectPtr> m_objects;
};

#endif /* MACRO_HPP */
