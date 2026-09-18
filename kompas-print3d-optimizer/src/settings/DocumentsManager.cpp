#include "DocumentsManager.hpp"

#include <unordered_map>

#include <KsAPI.h>

#include "settings/DocumentData.hpp"
#include "resources.hpp"

size_t IKompasDocumentPtrHash::operator()(ksapi::IKompasDocumentPtr document) const
{
    return std::hash<ksapi::IKompasDocument *>()(document.Get());
}

DocumentData& DocumentsManager::getOrCreateDocumentData(ksapi::IKompasDocumentPtr document)
{
    DocumentDataMap::iterator it = m_documentDataMap.find(document);
    if (it == m_documentDataMap.end()) {
        it = m_documentDataMap.emplace(std::piecewise_construct,
                                       std::forward_as_tuple(document),
                                       std::forward_as_tuple(document)
                                      ).first;

        auto onClose = [this, document]()
        {
            document->Events()->RemoveAllHandlers(resources::libraryName.data());
            m_documentDataMap.erase(document);
        };
        document->Events()->AddCloseDocumentHandler(resources::libraryName.data(), onClose);
    }
    return it->second;
}
