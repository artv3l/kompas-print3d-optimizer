#ifndef DOCUMENT_DATA_HPP
#define DOCUMENT_DATA_HPP

#include <unordered_map>
#include <string>

#include "PrintSurface.hpp"
#include "apiutil/Macro.hpp"
#include "NumericSetting.hpp"

class DocumentData {
public:
    class Settings {
    public:
        Settings(ksDocument3DPtr document3d);

        void refreshVariables() const;
        void setPrintSurface(PrintSurface printSurface);
        bool isPrintSurfaceSelected() const;
        PrintSurface getPrintSurface() const;
        NumericSetting::Ptr getSetting(std::string name);

    private:
        ksDocument3DPtr m_document3d;
        ksVariableCollectionPtr m_variableCollection;
        Optional<PrintSurface> m_printSurface;
        std::unordered_map<std::string, NumericSetting::Ptr> m_numericSettings;
    };

    static const char* ROOT_MACRO_NAME;

    DocumentData(ksDocument3DPtr document3d);

    Settings& getSettings();
    Macro getRootMacro();

private:
    ksPartPtr m_part;
    Settings m_settings;
    Macro m_rootMacro;

    static Macro getOrCreateRootMacro(ksPartPtr part);
};

#endif /* DOCUMENT_DATA_HPP */
