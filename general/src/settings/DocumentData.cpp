#include "stdafx.h"
#include "settings/DocumentData.hpp"

#include "apiutil/Macro.hpp"
#include "Optional.hpp"
#include "FrameEventImpl.hpp"
#include "settings/Settings.hpp"

const char* DocumentData::ROOT_MACRO_NAME = "Оптимизации";

IDocumentFramePtr getDocumentFrame(KompasObjectPtr kompas, ksDocument3DPtr document3d) {
    IKompasDocumentPtr document7 = kompas->TransferInterface(document3d, ksAPITypeEnum::ksAPI7Dual, 0);
    IDocumentFramesPtr documentFrames = document7->DocumentFrames;
    IDocumentFramePtr documentFrame = documentFrames->GetItem(0);
    return documentFrame;
}

DocumentData::DocumentData(KompasObjectPtr kompas, ksDocument3DPtr document3d):
    m_part(document3d->GetPart(pTop_Part)), m_settings(document3d), m_rootMacro(),
    m_frameEvent(getDocumentFrame(kompas, document3d), m_part, &m_settings)
{}

Settings& DocumentData::getSettings() {
    return m_settings;
}

Macro DocumentData::getOrCreateRootMacro() {
    if (!m_rootMacro || !m_rootMacro.value().isCreated() || (m_rootMacro.value().getName() != _bstr_t(ROOT_MACRO_NAME))) {
        ksEntityPtr macroEntity = Macro::findMacro(m_part, ROOT_MACRO_NAME);
        if (!macroEntity) {
            m_rootMacro = Macro(m_part, ROOT_MACRO_NAME, true);
        } else {
            m_rootMacro = Macro(macroEntity);
        }
    }
    return m_rootMacro.value();
}
