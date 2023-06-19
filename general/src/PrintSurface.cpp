#include "stdafx.h"
#include "PrintSurface.hpp"

#include <utility>
#include <stdexcept>

#include "utils.hpp"

PlaneEq::PlaneEq(ksFaceDefinitionPtr face) {
	ksSurfacePtr surface(face->GetSurface());
	double x0 = 0.0, y0 = 0.0, z0 = 0.0;
	surface->GetPoint(surface->GetParamUMax(), surface->GetParamVMax(), &x0, &y0, &z0);
	surface->GetNormal(surface->GetParamUMax(), surface->GetParamVMax(), &a_, &b_, &c_);
	d_ = -((a_ * x0) + (b_ * y0) + (c_ * z0));
}

bool PlaneEq::operator==(const PlaneEq& other) const {
	double scale = 0.0;
	if (!doubleEqual(other.a_, 0.0)) {
		scale = a_ / other.a_;
	} else if (!doubleEqual(other.b_, 0.0)) {
		scale = b_ / other.b_;
	} else if (!doubleEqual(other.c_, 0.0)) {
		scale = c_ / other.c_;
	}
	return doubleEqual(a_, other.a_ * scale) && doubleEqual(b_, other.b_ * scale) &&
		doubleEqual(c_, other.c_ * scale) && doubleEqual(d_, other.d_ * scale);
}

bool PlaneEq::operator!=(const PlaneEq& other) const {
	return !operator==(other);
}

void PlaneEq::invert() {
	a_ = -a_; b_ = -b_; c_ = -c_; d_ = -d_;
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
		double planeValue = (x * planeEq.a_) + (y * planeEq.b_) + (z * planeEq.c_) + planeEq.d_;
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

	if (nPointsOnEachSide.first == 0) {
		planeEq.invert();
		std::swap(nPointsOnEachSide.first, nPointsOnEachSide.second);
	}
	return PrintSurface{face, planeEq};
}
