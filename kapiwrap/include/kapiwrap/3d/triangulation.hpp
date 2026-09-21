#pragma once

#include <KsAPI.h>

#include "generic/geometry3d.hpp"

// Получить сетку всех полигональных объектов документа
geom3d::Mesh getMeshObjectsTriangulation(ksapi::IKompasDocument3DPtr document3d);
// Получить сетку тела
geom3d::Mesh getBodyTriangulation(ksapi::IBodyPtr body);
