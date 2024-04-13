#ifndef DOCUMENTS_MANAGER_HPP
#define DOCUMENTS_MANAGER_HPP

#include <unordered_map>

#include "settings/DocumentData.hpp"

struct Document3DPtrReferenceHash {
    size_t operator()(kapi::ksDocument3DPtr document3d) const;
};

class DocumentsManager {
public:
    DocumentsManager(kapi::KompasObjectPtr kompas);

    DocumentData& getOrCreateDocumentData(kapi::ksDocument3DPtr document3d);
    bool remove(kapi::ksDocument3DPtr document3d);

private:
    using DocumentDataMap = std::unordered_map<kapi::ksDocument3DPtr, DocumentData, Document3DPtrReferenceHash>;

    kapi::KompasObjectPtr m_kompas;
    DocumentDataMap m_documentDataMap;
};

#endif /* DOCUMENTS_MANAGER_HPP */
