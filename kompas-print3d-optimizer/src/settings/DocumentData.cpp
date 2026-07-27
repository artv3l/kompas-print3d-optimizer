#include "DocumentData.hpp"

#include "kapiwrap/Macro.hpp"
#include "settings/Settings.hpp"
#include "resources.hpp"

DocumentData::DocumentData(kapi::ksDocument3DPtr document3d, ksapi::IKompasDocumentPtr document):
    m_document3d(document3d),
    m_document(document),
    m_settings(),
    m_drawingManager(m_document->GetDocumentFrame(), resources::c_libraryName)
{
    m_settings.loadFromDocument(m_document3d);
}

Settings* DocumentData::getSettings() {
    return &m_settings;
}

DrawingManager& DocumentData::getDrawingManager()
{
    return m_drawingManager;
}

ksapi::IKompasDocumentPtr DocumentData::getDoc() const
{
    return m_document;
}
