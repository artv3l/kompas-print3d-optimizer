#pragma once

#include "generic/geometry3d.hpp"

geom3d::Mesh copyToMesh(kapi::ksTessellationPtr tessellation);
geom3d::Mesh copyToMesh(kapi::ksBodyPtr body);
