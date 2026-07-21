#pragma once

#include <optional>

#include "kapiwrap/Macro.hpp"
#include "settings/Settings.hpp"
#include "HighlightingManager.hpp"

class DocumentData {
public:
    DocumentData(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d);
    DocumentData(const DocumentData& obj) = delete;
    DocumentData(DocumentData&& obj) noexcept = delete;
    DocumentData& operator=(const DocumentData& obj) = delete;
    DocumentData& operator=(DocumentData&& obj) noexcept = delete;

    Settings* getSettings();
    HighlightingManager* getHighlightingManager();
    kapi::ksDocument3DPtr getDocument() const;

private:
    kapi::ksDocument3DPtr m_document3d;
    Settings m_settings;
    HighlightingManager m_highlightingManager;
};
