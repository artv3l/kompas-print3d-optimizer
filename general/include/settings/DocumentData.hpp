#ifndef DOCUMENT_DATA_HPP
#define DOCUMENT_DATA_HPP

#include <unordered_map>
#include <string>

#include "PrintSurface.hpp"
#include "apiutil/Macro.hpp"
#include "Setting.hpp"
#include "Optional.hpp"
#include "settings/Setting.hpp"
#include "apiutil/DocumentFrameEvent.hpp"

class DocumentData {
public:
    class Settings {
    public:
        Settings(ksDocument3DPtr document3d);

        void loadFromDocument();
        void uploadToDocument();
        void setPrintSurface(PrintSurface printSurface);
        bool isPrintSurfaceSelected() const;
        PrintSurface getPrintSurface() const;
        Setting::Ptr getSetting(std::string name);
        NumericSetting::Ptr getNumericSetting(std::string name);
        StringSetting::Ptr getStringSetting(std::string name);

    private:
        using SettingsMap = std::unordered_map<std::string, Setting::Ptr>;

        ksDocument3DPtr m_document3d;
        ksVariableCollectionPtr m_variableCollection;
        Optional<PrintSurface> m_printSurface;
        SettingsMap m_settingsMap;
    };

    static const char* ROOT_MACRO_NAME;

    DocumentData(KompasObjectPtr kompas, ksDocument3DPtr document3d);

    Settings& getSettings();
    Macro getOrCreateRootMacro();

private:
    ksPartPtr m_part;
    Settings m_settings;
    Optional<Macro> m_rootMacro;
    Optional<DocumentFrameEvent> m_documentFrameEvent;
};

#endif /* DOCUMENT_DATA_HPP */
