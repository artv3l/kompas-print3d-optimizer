#include "DocumentData.hpp"

#include "resources.hpp"

DocumentData::DocumentData(ksapi::IKompasDocumentPtr document):
    m_document(document),
    m_drawingManager(m_document->GetDocumentFrame(), resources::libraryName)
{
}

DrawingManager& DocumentData::getDrawingManager()
{
    return m_drawingManager;
}

ksapi::IKompasDocumentPtr DocumentData::getDoc() const
{
    return m_document;
}
