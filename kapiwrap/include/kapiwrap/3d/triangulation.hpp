#pragma once

#include <KsAPI.h>

#include "generic/geometry3d.hpp"

// Получить сетку детали: все грани тел и поверхностей
geom3d::Mesh copyToMesh(ksapi::IPartPtr part);
// Получить сетку всех полигональных объектов документа
geom3d::Mesh getMeshObjectsTriangulation(ksapi::IKompasDocument3DPtr document3d);
