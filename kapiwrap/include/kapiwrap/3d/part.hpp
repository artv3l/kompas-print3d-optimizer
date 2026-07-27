#pragma once

#include <KsAPI.h>

#include "generic/geometry3d.hpp"

std::vector<ksapi::IFacePtr> getFaces(ksapi::IPartPtr part);
std::vector<ksapi::IEdgePtr> getEdges(ksapi::IFacePtr face);
geom3d::Mesh copyToMesh(ksapi::IPartPtr part);
geom3d::Gabarit getGabarit(ksapi::IPartPtr part);
