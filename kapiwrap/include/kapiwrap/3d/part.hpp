#pragma once

#include <KsAPI.h>

std::vector<ksapi::IFacePtr> getFaces(ksapi::IPartPtr part);
std::vector<ksapi::IEdgePtr> getEdges(ksapi::IFacePtr face);
