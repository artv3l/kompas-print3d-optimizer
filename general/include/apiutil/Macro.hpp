#ifndef MACRO_HPP
#define MACRO_HPP

#include <comutil.h>

class Macro {
public:
    Macro(ksPartPtr part, _bstr_t name, bool staffVisible);
    explicit Macro(ksEntityPtr entity);

    ~Macro();
    
    static ksEntityPtr findMacro(ksPartPtr part, _bstr_t name);

    bool add(IDispatchPtr obj);
    bool add(Macro macro);
    bool update();
    bool isCreated() const;
    _bstr_t getName() const;

private:
    ksEntityPtr entity_;
    ksMacro3DDefinitionPtr definition_;

};

#endif /* MACRO_HPP */
