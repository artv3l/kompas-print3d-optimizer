#include "DocumentData.hpp"

#include "kapiwrap/Macro.hpp"
#include "HighlightingManager.hpp"
#include "settings/Settings.hpp"
#include "resources.hpp"

DocumentData::DocumentData(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d, ksapi::IKompasDocumentPtr document):
    m_document3d(document3d),
    m_document(document),
    m_settings(),
    m_highlightingManager(kompas, document3d, &m_settings),
    m_drawingManager(m_document->GetDocumentFrame(), resources::c_libraryName)
{
    m_settings.loadFromDocument(m_document3d);
}

Settings* DocumentData::getSettings() {
    return &m_settings;
}

HighlightingManager* DocumentData::getHighlightingManager() {
    return &m_highlightingManager;
}

DrawingManager& DocumentData::getDrawingManager()
{
    return m_drawingManager;
}

kapi::ksDocument3DPtr DocumentData::getDocument() const
{
    return m_document3d;
}

ksapi::IKompasDocumentPtr DocumentData::getDoc() const
{
    return m_document;
}
