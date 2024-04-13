#ifndef MACRO_HPP
#define MACRO_HPP

#include <comutil.h>

class Macro {
public:
    Macro(kapi::ksPartPtr part, _bstr_t name, bool staffVisible);
    explicit Macro(kapi::ksEntityPtr entity);

    ~Macro();
    
    static kapi::ksEntityPtr findMacro(kapi::ksPartPtr part, _bstr_t name);

    bool add(IDispatchPtr obj);
    bool add(Macro macro);
    bool update();
    bool isCreated() const;
    _bstr_t getName() const;
    kapi::ksEntityPtr getEntity() const;

private:
    kapi::ksEntityPtr m_entity;
    kapi::ksMacro3DDefinitionPtr m_definition;

};

#endif /* MACRO_HPP */
