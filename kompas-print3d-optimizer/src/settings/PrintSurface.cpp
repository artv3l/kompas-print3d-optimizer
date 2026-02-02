#include "PrintSurface.hpp"

#include <utility>
#include <stdexcept>
#include <span>
#include <algorithm>

#include <glm/glm.hpp>

#include "utils.hpp"

PlaneEq::PlaneEq(kapi::ksFaceDefinitionPtr face) {
	if (!face->IsPlanar()) {
		throw std::runtime_error("The face is not planar");
	}
	kapi::ksSurfacePtr surface(face->GetSurface());
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

std::pair<int, int> countPointsOnEachSide(kapi::ksPartPtr part, const PlaneEq& planeEq) {
	kapi::ksEntityCollectionPtr entityCollection(part->EntityCollection(kapi::o3d_vertex));

	int s1 = 0, s2 = 0;
	int nEntities = entityCollection->GetCount();
	for (int iEntity = 0; iEntity < nEntities; iEntity++) {
		kapi::ksEntityPtr entity(entityCollection->GetByIndex(iEntity));
		kapi::ksVertexDefinitionPtr vertex(entity->GetDefinition());
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

PrintSurface getSelectedPrintSurface(kapi::ksDocument3DPtr document3d) {
	kapi::ksSelectionMngPtr selectionMng(document3d->GetSelectionMng());

	if (selectionMng->GetCount() == 0) {
		throw std::runtime_error("Плоскость печати не выбрана!");
	}
	if (selectionMng->GetCount() != 1) {
		throw std::runtime_error("Должен был быть выбран только один элемент в виде плоской грани!");
	}
	kapi::ksEntityPtr entity = selectionMng->GetObjectByIndex(0);
	if (entity->type != kapi::Obj3dType::o3d_face) {
		throw std::runtime_error("Выбранный элемент не является гранью!");
	}

	kapi::ksFaceDefinitionPtr face(entity->GetDefinition());
	if (!face->IsPlanar()) {
		throw std::runtime_error("Выбранная грань должна быть плоской!");
	}

	PlaneEq planeEq(face);
	std::pair<int, int> nPointsOnEachSide = countPointsOnEachSide(document3d->GetPart(kapi::pTop_Part), planeEq);
	if ((nPointsOnEachSide.first != 0) && ((nPointsOnEachSide.second != 0))) {
		throw std::runtime_error("Плоскость печати пересекает деталь!");
	}

	return PrintSurface{face, planeEq};
}

using Lock = std::function<void()>;
template <typename T>
std::pair<std::span<T>, Lock> getSafeArrayData(const _variant_t& variant)
{
	if (!(variant.vt & VT_ARRAY) || !variant.parray) {
		assert(false);
		return {};
	}

	size_t count = variant.parray->rgsabound[0].cElements - variant.parray->rgsabound[0].lLbound;
	if (count <= 0 || variant.parray->cDims != 1) {
		assert(false);
		return {};
	}

	T HUGEP * data = nullptr;
	SafeArrayAccessData(variant.parray, (void HUGEP * FAR*) & data);

	const UINT elemSize = SafeArrayGetElemsize(variant.parray);
	return { std::span<T>(data, count / (sizeof(T) / elemSize)), [&variant](){ SafeArrayUnaccessData(variant.parray); }};
}


OrientationStat calcOrientationStat(kapi::ksBodyPtr body, const glm::vec3& direction)
{
	auto calcTriangleArea = [](const glm::dvec3& a, const glm::dvec3& b, const glm::dvec3& c) {
		glm::dvec3 ab = b - a;
		glm::dvec3 ac = c - a;
		return 0.5 * glm::length(glm::cross(ab, ac));
	};

	auto calcAngle = [](const glm::vec3 & a, const glm::vec3 & b) -> float {
		float dot = glm::dot(a, b);
		float len = glm::length(a) * glm::length(b);
		if (doubleEqual(len, 0.0))
			return 0.0f;
		return std::acos(std::clamp(dot / len, -1.0f, 1.0f));
	};

	checkPtr(body);
	auto faces = checkCast<kapi::ksFaceCollectionPtr>(body->FaceCollection());

	OrientationStat result;

	for (size_t i = 0, facesCount = faces->GetCount(); i < facesCount; ++i) {
		kapi::ksFaceDefinitionPtr face = faces->GetByIndex(i);
		kapi::ksTessellationPtr tessellation = face->GetTessellation();
		tessellation->refresh(); // Нужно обязательно вызывать после перестроения модели

		_variant_t pointsVariant, indexesVariant, normalsVariant;
		tessellation->GetFacetPoints(&pointsVariant, &indexesVariant);
		tessellation->GetFacetNormals(&normalsVariant);
		auto&& [normals, normalsLock] = getSafeArrayData<glm::dvec3>(normalsVariant);
		auto&& [points, pointsLock] = getSafeArrayData<glm::dvec3>(pointsVariant);
		auto&& [indexes, indexesLock] = getSafeArrayData<int>(indexesVariant);

		for (int i = 0; i < indexes.size(); i += 3) {
			glm::dvec3 normal = normals[indexes[i]];
			double area = calcTriangleArea(points[indexes[i]], points[indexes[i + 1]], points[indexes[i + 2]]);
			result.bodyArea += area;
			if (calcAngle(normal, direction) < degreeToRadian(45)) {
				result.supportArea += area;
			}
		}
	}

	return result;
}
