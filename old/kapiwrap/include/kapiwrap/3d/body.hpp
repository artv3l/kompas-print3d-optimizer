#pragma once

#include <KsAPI.h>

#include "generic/geometry3d.hpp"
#include "kapiwrap/Sketch.hpp"

ksapi::IEvolutionPtr createEvolution(ksapi::IPartPtr part, const Sketch & sketch, ksEvolutionShiftSketchTypeEnum sketchShiftType, std::vector<ksapi::IModelObjectPtr> edges);
