#ifndef SETTINGS_MANAGER_HPP
#define SETTINGS_MANAGER_HPP

#include <utility>
#include <unordered_map>
#include <memory>

#include "apiutil/PropertyManagerObject.hpp"
#include "PrintSurface.hpp"
#include "Optional.hpp"

struct Settings {
    Optional<PrintSurface> printSurface;

    double layerHeight;
    double overhangThreshold;

    double roundingRadius;
    double roundingDeflectionAngle;

    uint8_t elephantFootLayersCount;

    uint8_t bridgeHoleFillLayersCount;
    uint8_t bridgeHoleBuildLayersCount;

    Settings();
    Settings(const PrintSurface& printSurface_);
};

class SettingsManager : public PropertyManagerObject {
public:
    SettingsManager(KompasObjectPtr kompas);
    virtual ~SettingsManager() = default;

    void show() override;

    void setPrintSurface(ksDocument3DPtr document3d, const PrintSurface& printSurface);
    Settings* getSettings(ksDocument3DPtr document3d);

private:
    using DocumentSettingsMap = std::unordered_map<ksDocument3D*, Settings>;

    DocumentSettingsMap mapDocumentSettings_;
    IPropertyTabPtr mainTab_;
    IPropertyControlsPtr controls_;

    struct {
        IPropertyEditPtr layerHeight;
        IPropertyEditPtr overhangThreshold;

        IPropertyEditPtr roundingRadius;
        IPropertyEditPtr roundingDeflectionAngle;

        IPropertyEditPtr elephantFootLayersCount;

        IPropertyEditPtr bridgeHoleFillLayersCount;
        IPropertyEditPtr bridgeHoleBuildLayersCount;
    } edits_;

    virtual bool buttonClick(long buttonId) override;

    void initControls();
    void fillSettingsToEdits();
};

#endif /* SETTINGS_MANAGER_HPP */
