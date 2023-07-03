#ifndef DOCUMENTS_MANAGER_HPP
#define DOCUMENTS_MANAGER_HPP

#include <unordered_map>

#include "apiutil/Macro.hpp"
#include "Optional.hpp"
#include "DocumentData.hpp"

class DocumentsManager {
public:
    DocumentData& getOrCreateDocumentData(ksDocument3DPtr document3d);

private:
    using DocumentDataMap = std::unordered_map<ksDocument3D*, DocumentData>;

    DocumentDataMap m_documentDataMap;
};

#endif /* DOCUMENTS_MANAGER_HPP */
