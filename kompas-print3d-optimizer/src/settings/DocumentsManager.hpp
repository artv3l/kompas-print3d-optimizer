#pragma once

#include <unordered_map>

#include "settings/DocumentData.hpp"

struct IKompasDocumentPtrHash
{
    size_t operator()(ksapi::IKompasDocumentPtr document) const;
};

class DocumentsManager
{
public:
    DocumentsManager(kapi::KompasObjectPtr kompas);

    DocumentData& getOrCreateDocumentData(ksapi::IKompasDocumentPtr document, kapi::ksDocument3DPtr document3d);

private:
    using DocumentDataMap = std::unordered_map<ksapi::IKompasDocumentPtr, DocumentData, IKompasDocumentPtrHash>;

    kapi::KompasObjectPtr m_kompas;
    DocumentDataMap m_documentDataMap;
};
