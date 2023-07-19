#ifndef DOCUMENT_DATA_HPP
#define DOCUMENT_DATA_HPP

#include "apiutil/Macro.hpp"
#include "Optional.hpp"
#include "Settings.hpp"
#include "FrameEventImpl.hpp"

class DocumentData {
public:
    static const char* ROOT_MACRO_NAME;

    DocumentData(KompasObjectPtr kompas, ksDocument3DPtr document3d);
    DocumentData(const DocumentData& other) = delete;
    DocumentData(DocumentData&& other) noexcept = delete;

    Settings& getSettings();
    Macro getOrCreateRootMacro();

private:
    ksDocument3DPtr m_document3d;
    Settings m_settings;
    Optional<Macro> m_rootMacro;
    FrameEventImpl m_frameEvent;
};

#endif /* DOCUMENT_DATA_HPP */
