#ifndef DOCUMENT_DATA_HPP
#define DOCUMENT_DATA_HPP

#include "kapiwrap/Macro.hpp"
#include "Optional.hpp"
#include "settings/Settings.hpp"
#include "HighlightingManager.hpp"

class DocumentData {
public:
    static const char* ROOT_MACRO_NAME;

    DocumentData(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d);
    DocumentData(const DocumentData& obj) = delete;
    DocumentData(DocumentData&& obj) noexcept = delete;
    DocumentData& operator=(const DocumentData& obj) = delete;
    DocumentData& operator=(DocumentData&& obj) noexcept = delete;

    Settings* getSettings();
    HighlightingManager* getHighlightingManager();
    Macro getOrCreateRootMacro();
    kapi::ksDocument3DPtr getDocument() const;

private:
    kapi::ksDocument3DPtr m_document3d;
    Settings m_settings;
    Optional<Macro> m_rootMacro;
    HighlightingManager m_highlightingManager;
};

#endif /* DOCUMENT_DATA_HPP */
