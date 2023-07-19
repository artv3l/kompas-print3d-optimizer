#include "stdafx.h"
#include "settings/DocumentData.hpp"

#include "apiutil/Macro.hpp"
#include "Optional.hpp"
#include "FrameEventImpl.hpp"
#include "settings/Settings.hpp"

const char* DocumentData::ROOT_MACRO_NAME = "Оптимизации";

DocumentData::DocumentData(KompasObjectPtr kompas, ksDocument3DPtr document3d):
    m_document3d(document3d), m_settings(document3d), m_rootMacro(),
    m_frameEvent(kompas, document3d, &m_settings)
{}

Settings& DocumentData::getSettings() {
    return m_settings;
}

Macro DocumentData::getOrCreateRootMacro() {
    ksPartPtr part = m_document3d->GetPart(Part_Type::pTop_Part);
    if (!m_rootMacro || !m_rootMacro.value().isCreated() || (m_rootMacro.value().getName() != _bstr_t(ROOT_MACRO_NAME))) {
        ksEntityPtr macroEntity = Macro::findMacro(part, ROOT_MACRO_NAME);
        if (!macroEntity) {
            m_rootMacro = Macro(part, ROOT_MACRO_NAME, true);
        } else {
            m_rootMacro = Macro(macroEntity);
        }
    }
    return m_rootMacro.value();
}
