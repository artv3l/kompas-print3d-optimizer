#include "DocumentData.hpp"

#include "kapiwrap/Macro.hpp"
#include "Optional.hpp"
#include "HighlightingManager.hpp"
#include "settings/Settings.hpp"

const char* DocumentData::ROOT_MACRO_NAME = "Оптимизации";

DocumentData::DocumentData(KompasObjectPtr kompas, ksDocument3DPtr document3d):
    m_document3d(document3d), m_settings(), m_rootMacro(),
    m_highlightingManager(kompas, document3d, &m_settings)
{
    m_settings.loadFromDocument(m_document3d);
}

Settings* DocumentData::getSettings() {
    return &m_settings;
}

HighlightingManager* DocumentData::getHighlightingManager() {
    return &m_highlightingManager;
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
