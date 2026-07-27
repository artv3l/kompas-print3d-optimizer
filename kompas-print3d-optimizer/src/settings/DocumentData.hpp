#pragma once

#include <optional>

#include "kapiwrap/Macro.hpp"
#include "drawing/DrawingManager.hpp"
#include "settings/Settings.hpp"
#include "HighlightingManager.hpp"

class DocumentData {
public:
    DocumentData(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d, ksapi::IKompasDocumentPtr document);
    DocumentData(const DocumentData& obj) = delete;
    DocumentData(DocumentData&& obj) noexcept = delete;
    DocumentData& operator=(const DocumentData& obj) = delete;
    DocumentData& operator=(DocumentData&& obj) noexcept = delete;

    Settings* getSettings();
    HighlightingManager* getHighlightingManager();
    DrawingManager& getDrawingManager();
    kapi::ksDocument3DPtr getDocument() const;
    ksapi::IKompasDocumentPtr getDoc() const;

private:
    kapi::ksDocument3DPtr m_document3d;
    ksapi::IKompasDocumentPtr m_document;
    Settings m_settings;
    HighlightingManager m_highlightingManager;
    DrawingManager m_drawingManager;
};
