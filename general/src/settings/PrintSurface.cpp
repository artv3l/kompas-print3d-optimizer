#include "stdafx.h"
#include "settings/PrintSurface.hpp"

#include <utility>
#include <stdexcept>

#include "utils.hpp"

PlaneEq::PlaneEq(ksFaceDefinitionPtr face) {
	if (!face->IsPlanar()) {
		throw std::runtime_error("The face is not planar");
	}
	ksSurfacePtr surface(face->GetSurface());
	double x0 = 0.0, y0 = 0.0, z0 = 0.0;
	surface->GetPoint(surface->GetParamUMax(), surface->GetParamVMax(), &x0, &y0, &z0);
	surface->GetNormal(surface->GetParamUMax(), surface->GetParamVMax(), &a, &b, &c);
	d = -((a * x0) + (b * y0) + (c * z0));

	if (!face->normalOrientation) {
		invert();
	}
}

bool PlaneEq::operator==(const PlaneEq& other) const {
	double scale = 0.0;
	if (!doubleEqual(other.a, 0.0)) {
		scale = a / other.a;
	} else if (!doubleEqual(other.b, 0.0)) {
		scale = b / other.b;
	} else if (!doubleEqual(other.c, 0.0)) {
		scale = c / other.c;
	}
	return doubleEqual(a, other.a * scale) && doubleEqual(b, other.b * scale) &&
		doubleEqual(c, other.c * scale) && doubleEqual(d, other.d * scale);
}

bool PlaneEq::operator!=(const PlaneEq& other) const {
	return !operator==(other);
}

void PlaneEq::invert() {
	a = -a; b = -b; c = -c; d = -d;
}

std::pair<int, int> countPointsOnEachSide(ksPartPtr part, const PlaneEq& planeEq) {
	ksEntityCollectionPtr entityCollection(part->EntityCollection(o3d_vertex));

	int s1 = 0, s2 = 0;
	int nEntities = entityCollection->GetCount();
	for (int iEntity = 0; iEntity < nEntities; iEntity++) {
		ksEntityPtr entity(entityCollection->GetByIndex(iEntity));
		ksVertexDefinitionPtr vertex(entity->GetDefinition());
		if (!vertex->topologyVertex) {
			continue;
		}

		double x, y, z;
		vertex->GetPoint(&x, &y, &z);
		double planeValue = (x * planeEq.a) + (y * planeEq.b) + (z * planeEq.c) + planeEq.d;
		if (doubleEqual(planeValue, 0.0)) {
			continue;
		}
		if (planeValue > 0.0) {
			s1++;
		} else {
			s2++;
		}
	}
	return std::make_pair(s1, s2);
}

PrintSurface getSelectedPrintSurface(ksDocument3DPtr document3d) {
	ksSelectionMngPtr selectionMng(document3d->GetSelectionMng());

	if (selectionMng->GetCount() == 0) {
		throw std::runtime_error("Плоскость печати не выбрана!");
	}
	if (selectionMng->GetCount() != 1) {
		throw std::runtime_error("Должен был быть выбран только один элемент в виде плоской грани!");
	}
	ksEntityPtr entity = selectionMng->GetObjectByIndex(0);
	if (entity->type != Obj3dType::o3d_face) {
		throw std::runtime_error("Выбранный элемент не является гранью!");
	}

	ksFaceDefinitionPtr face(entity->GetDefinition());
	if (!face->IsPlanar()) {
		throw std::runtime_error("Выбранная грань должна быть плоской!");
	}

	PlaneEq planeEq(face);
	std::pair<int, int> nPointsOnEachSide = countPointsOnEachSide(document3d->GetPart(pTop_Part), planeEq);
	if ((nPointsOnEachSide.first != 0) && ((nPointsOnEachSide.second != 0))) {
		throw std::runtime_error("Плоскость печати пересекает деталь!");
	}

	return PrintSurface{face, planeEq};
}
