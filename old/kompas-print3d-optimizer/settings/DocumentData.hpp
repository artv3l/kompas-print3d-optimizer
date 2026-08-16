#pragma once

#include <optional>

#include "kapiwrap/Macro.hpp"
#include "drawing/DrawingManager.hpp"
#include "settings/Settings.hpp"

class DocumentData {
public:
    DocumentData(kapi::ksDocument3DPtr document3d, ksapi::IKompasDocumentPtr document);
    DocumentData(const DocumentData& obj) = delete;
    DocumentData(DocumentData&& obj) noexcept = delete;
    DocumentData& operator=(const DocumentData& obj) = delete;
    DocumentData& operator=(DocumentData&& obj) noexcept = delete;

    Settings* getSettings();
    DrawingManager& getDrawingManager();
    ksapi::IKompasDocumentPtr getDoc() const;

private:
    kapi::ksDocument3DPtr m_document3d;
    ksapi::IKompasDocumentPtr m_document;
    Settings m_settings;
    DrawingManager m_drawingManager;
};
