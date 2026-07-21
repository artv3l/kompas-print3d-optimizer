#include "DocumentData.hpp"

#include "kapiwrap/Macro.hpp"
#include "HighlightingManager.hpp"
#include "settings/Settings.hpp"

DocumentData::DocumentData(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d):
    m_document3d(document3d), m_settings(),
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

kapi::ksDocument3DPtr DocumentData::getDocument() const
{
    return m_document3d;
}
