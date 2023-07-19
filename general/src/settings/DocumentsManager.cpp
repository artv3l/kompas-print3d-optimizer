#include "stdafx.h"
#include "settings/DocumentsManager.hpp"

#include "settings/DocumentData.hpp"

DocumentsManager::DocumentsManager(KompasObjectPtr kompas):
    m_kompas(kompas)
{}

DocumentData& DocumentsManager::getOrCreateDocumentData(ksDocument3DPtr document3d) {
    DocumentDataMap::iterator it = m_documentDataMap.find(document3d);
    if (it == m_documentDataMap.end()) {
        it = m_documentDataMap.emplace(std::piecewise_construct,
                                       std::forward_as_tuple((ksDocument3D*)document3d),
                                       std::forward_as_tuple(m_kompas, document3d)
                                      ).first;
    } else {
        it->second.getSettings().loadFromDocument();
    }
    return it->second;
}
