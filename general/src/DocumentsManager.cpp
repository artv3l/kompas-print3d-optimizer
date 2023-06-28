#include "stdafx.h"
#include "DocumentsManager.hpp"

DocumentData* DocumentsManager::getOrCreateDocumentData(ksDocument3DPtr document3d) {
    DocumentDataMap::iterator it = documentDataMap_.find(document3d);
    if (it == documentDataMap_.end()) {
        it = documentDataMap_.insert(std::make_pair(document3d, DocumentData{})).first;
    }
    return &it->second;
}
