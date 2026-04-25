#pragma once

#include <chrono>

#include "ActionLock.hpp"

namespace perfomance
{
// Р¤СѓРЅРєС†РёСЏ, РѕР±СЂР°Р±Р°С‚С‹РІР°СЋС‰Р°СЏ СЂРµР·СѓР»СЊС‚Р°С‚ РёР·РјРµСЂРµРЅРёСЏ РІСЂРµРјРµРЅРё СЂР°Р±РѕС‚С‹ РєРѕРґР°
using MeasureTimeFunc = std::function<void(std::chrono::nanoseconds)>;

// Р—Р°РјРµСЂРёС‚СЊ РІСЂРµРјСЏ СЂР°Р±РѕС‚С‹ РєРѕРґР°
ActionLock measureTime(MeasureTimeFunc func);
}
