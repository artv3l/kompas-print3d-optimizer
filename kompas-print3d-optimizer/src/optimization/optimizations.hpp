#pragma once

#include <KsAPI.h>

class Settings;

enum class ReworkType {
    ALL,
    ONLY_WITH_REWORK,
    ONLY_WITHOUT_REWORK,
};

ksapi::IModelObjectPtr optimizeElephantFoot(ksapi::IPartPtr part, Settings& settings);
ksapi::IModelObjectPtr optimizeRoundingEdgesOnPrintFace(ksapi::IPartPtr part, Settings& settings, ReworkType reworkType, size_t& reworkCount);
