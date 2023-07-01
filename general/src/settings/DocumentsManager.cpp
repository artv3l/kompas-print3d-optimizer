#include "stdafx.h"
#include "settings/DocumentsManager.hpp"

#include "settings/DocumentData.hpp"

DocumentData& DocumentsManager::getOrCreateDocumentData(ksDocument3DPtr document3d) {
    DocumentDataMap::iterator it = documentDataMap_.find(document3d);
    if (it == documentDataMap_.end()) {
        it = documentDataMap_.insert(std::make_pair(document3d, DocumentData(document3d))).first;
    }
    return it->second;
}
