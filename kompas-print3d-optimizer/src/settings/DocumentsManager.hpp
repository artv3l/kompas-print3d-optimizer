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
    DocumentData& getOrCreateDocumentData(ksapi::IKompasDocumentPtr document);

private:
    using DocumentDataMap = std::unordered_map<ksapi::IKompasDocumentPtr, DocumentData, IKompasDocumentPtrHash>;

    DocumentDataMap m_documentDataMap;
};
