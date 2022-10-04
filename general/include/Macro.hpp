#ifndef MACRO_HPP
#define MACRO_HPP

#include "stdafx.h"

#include <comutil.h>

class Macro {
public:
    Macro(ksPartPtr part, _bstr_t name, bool staffVisible);

    ~Macro();
    
    bool add(IDispatchPtr obj);
    bool add(Macro macro);
    bool update();

private:
    ksEntityPtr entity_;
    ksMacro3DDefinitionPtr definition_;

};

#endif /* MACRO_HPP */
