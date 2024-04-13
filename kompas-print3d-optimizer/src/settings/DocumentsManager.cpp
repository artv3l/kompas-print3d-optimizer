#include "DocumentsManager.hpp"

#include <unordered_map>

#include "settings/DocumentData.hpp"
#include "settings/Settings.hpp"

size_t Document3DPtrReferenceHash::operator()(kapi::ksDocument3DPtr document3d) const {
    return document3d->reference;
}

DocumentsManager::DocumentsManager(kapi::KompasObjectPtr kompas):
    m_kompas(kompas)
{}

DocumentData& DocumentsManager::getOrCreateDocumentData(kapi::ksDocument3DPtr document3d) {
    DocumentDataMap::iterator it = m_documentDataMap.find(document3d);
    if (it == m_documentDataMap.end()) {
        it = m_documentDataMap.emplace(std::piecewise_construct,
                                       std::forward_as_tuple(document3d),
                                       std::forward_as_tuple(m_kompas, document3d)
                                      ).first;
    } else {
        it->second.getSettings()->loadFromDocument(document3d);
    }
    return it->second;
}

bool DocumentsManager::remove(kapi::ksDocument3DPtr document3d) {
    DocumentDataMap::iterator it = m_documentDataMap.find(document3d);
    if (it == m_documentDataMap.end()) {
        return false;
    } else {
        m_documentDataMap.erase(it);
        return true;
    }
}
