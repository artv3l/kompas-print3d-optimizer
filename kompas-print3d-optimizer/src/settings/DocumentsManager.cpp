#include "DocumentsManager.hpp"

#include <unordered_map>

#include <KsAPI.h>

#include "settings/DocumentData.hpp"
#include "settings/Settings.hpp"
#include "resources.hpp"

size_t IKompasDocumentPtrHash::operator()(ksapi::IKompasDocumentPtr document) const
{
    return std::hash<ksapi::IKompasDocument *>()(document.Get());
}

DocumentsManager::DocumentsManager(kapi::KompasObjectPtr kompas):
    m_kompas(kompas)
{}

DocumentData& DocumentsManager::getOrCreateDocumentData(ksapi::IKompasDocumentPtr document, kapi::ksDocument3DPtr document3d)
{
    DocumentDataMap::iterator it = m_documentDataMap.find(document);
    if (it == m_documentDataMap.end()) {
        it = m_documentDataMap.emplace(std::piecewise_construct,
                                       std::forward_as_tuple(document),
                                       std::forward_as_tuple(m_kompas, document3d)
                                      ).first;

        auto onClose = [this, document]()
        {
            document->Events()->RemoveAllHandlers(resources::c_libraryName.data());
            m_documentDataMap.erase(document);
        };
        document->Events()->AddCloseDocumentHandler(resources::c_libraryName.data(), onClose);
    } else {
        it->second.getSettings()->loadFromDocument(document3d);
    }
    return it->second;
}
