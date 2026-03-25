#include "PrintSurface.hpp"

#include <utility>
#include <stdexcept>
#include <span>
#include <algorithm>
#include <iterator>
#include <optional>
#include <ranges>
#include <numeric>

#include <glm/glm.hpp>

#include "utils.hpp"
#include "generic/math.hpp"
#include "windows.hpp"
#include "mesh.hpp"

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

namespace
{
OrientationInfo calcOrientationInfo(const Mesh& mesh, const glm::vec3& direction, double overhangThreshold, double offsetThreshold)
{
	OrientationInfo info;

	const double overhangThresholdRad = degreeToRadian(overhangThreshold);
	const auto [printPlane, height] = calcPrintPlaneAndHeight(mesh, direction);

	const math::Placement printPlanePlacement = math::Placement::createByAxisZ(
		math::project(glm::vec3(0, 0, 0), printPlane), printPlane.getNormal()
	);
	const glm::mat4 toWorld = printPlanePlacement.matrixToWorld();
	const glm::mat4 toPrintPlanePlacement = math::worldToLocal(printPlanePlacement);

	std::vector<glm::vec2> convexHullPoints;
	convexHullPoints.reserve(mesh.positions.size());

	for (int iIndex = 0; (iIndex + 2) < mesh.indexes.size(); iIndex += 3) {
		const size_t i1 = mesh.indexes[iIndex];
		const size_t i2 = mesh.indexes[iIndex + 1];
		const size_t i3 = mesh.indexes[iIndex + 2];
		const math::Triangle triangle(mesh.positions[i1], mesh.positions[i2], mesh.positions[i3]);

		// Для всех трех точек будут одинаковые нормали, поэтому берем любую (первую)
		const glm::vec3 normal = mesh.normals[i1];
		const double angleRad = calcAngleBetween(normal, printPlane.getNormal());
		const double triangleArea = triangle.area();

		if (isOnPrintPlane(triangle, printPlane, offsetThreshold)) {
			info.bottomArea += triangleArea;
		}
		else if (angleRad < overhangThresholdRad) {
			// Площадь под мостами тоже считаем за площадь нависаний
			info.overhangArea += triangleArea;
		    info.overhangVolume += volumeUnderOverhang(printPlane, triangle);
		}

		for (auto&& pnt : triangle.points) {
			const glm::vec4 pntLocal = toPrintPlanePlacement * glm::vec4(pnt, 1.0f);
			if (std::abs(pntLocal.z) < offsetThreshold) {
				convexHullPoints.emplace_back(pntLocal.x, pntLocal.y);
			}
		}
	}

	info.modelHeight = height;

	if (convexHullPoints.size() >= 3) {
		geometry::Polygon hullPolygon = convexHull(convexHullPoints);
		info.bottomConvexHullArea = hullPolygon.area();

		for (auto&& pnt : hullPolygon.m_points) {
			info.bottomContour.push_back(toWorld * glm::vec4(pnt.x, pnt.y, 0.0f, 1.0f));
		}
	}
	else {
		for (auto&& pnt : convexHullPoints) {
			info.bottomContour.push_back(toWorld * glm::vec4(pnt.x, pnt.y, 0.0f, 1.0f));
		}
	}

	return info;
}

// Рассчитать все критерии для нескольких вариантов ориентации
std::vector<OrientationInfo> calcOrientationsEstimation(const Mesh& mesh, std::span<const glm::vec3> directions, double overhangThreshold, double offsetThreshold)
{
	assert(mesh.indexes.size() % 3 == 0);

	std::vector<OrientationInfo> result;
	result.resize(directions.size());
	for (size_t i = 0; i < directions.size(); ++i) {
		result[i] = calcOrientationInfo(mesh, directions[i], overhangThreshold, offsetThreshold);
	}
	return result;
}

// Преобразовать абсолюные значения в относительные [0, 1]
template <std::ranges::range R>
std::vector<double> toRelative(R absoluteValues, bool invert)
{
	std::vector<double> relativeValues(absoluteValues.size(), 0.0);

	const auto [min, max] = std::ranges::minmax_element(absoluteValues);
	if (min == absoluteValues.end() || max == absoluteValues.end())
		throw std::logic_error(""); // TODO

	auto convert = std::bind(math::convertRanges, std::placeholders::_1, *min, *max, 0.0, 1.0);
	std::ranges::transform(absoluteValues, relativeValues.begin(), convert);

	if (invert)
		std::ranges::transform(relativeValues, relativeValues.begin(), [](auto value) {return 1.0 - value;});

	return relativeValues;
}

// Рассчитать все составные критерии
OrientationComplexInfos calcOrientationsComplexEstimation(std::span<OrientationInfo> infos)
{
	OrientationComplexInfos result;

	// TODO
	result[enums::toUnderlying(OrientationComplexCriteria::overhangs)] = toRelative(infos | std::views::transform(&OrientationInfo::overhangArea), false);
	result[enums::toUnderlying(OrientationComplexCriteria::bottomQuality)] = toRelative(infos | std::views::transform(&OrientationInfo::bottomArea), true);
	result[enums::toUnderlying(OrientationComplexCriteria::common)] = std::vector<double>(infos.size(), 0.0);

	return result;
}
}

Mesh copyToMesh(kapi::ksTessellationPtr tessellation)
{
	checkPtr(tessellation);

	tessellation->refresh();

	_variant_t pointsVariant, indexesVariant, normalsVariant;
	tessellation->GetFacetPoints(&pointsVariant, &indexesVariant);
	tessellation->GetFacetNormals(&normalsVariant);
	auto&& [points, pointsLock] = getSafeArrayData<glm::dvec3>(pointsVariant);
	auto&& [normals, normalsLock] = getSafeArrayData<glm::dvec3>(normalsVariant);
	auto&& [indexes, indexesLock] = getSafeArrayData<int>(indexesVariant);

	auto toFloatVec = [](const glm::dvec3& dvec3) { return glm::vec3(dvec3); };

	Mesh mesh;

	mesh.positions.reserve(points.size());
	std::transform(points.begin(), points.end(), std::back_inserter(mesh.positions), toFloatVec);

	mesh.normals.reserve(normals.size());
	std::transform(normals.begin(), normals.end(), std::back_inserter(mesh.normals), toFloatVec);

	std::copy(indexes.begin(), indexes.end(), std::back_inserter(mesh.indexes));

	return mesh;
}

Mesh copyToMesh(kapi::ksBodyPtr body)
{
	auto faces = checkCast<kapi::ksFaceCollectionPtr>(checkPtr(body)->FaceCollection());

	Mesh mesh;

	for (long iFace = 0, nFaces = faces->GetCount(); iFace < nFaces; ++iFace) {
		kapi::ksFaceDefinitionPtr face = checkPtr(faces->GetByIndex(iFace));
		kapi::ksTessellationPtr tessellation = checkPtr(face->GetTessellation());

		if (iFace == 0) {
			mesh = copyToMesh(tessellation);
		} else {
			Mesh faceMesh = copyToMesh(tessellation);
			const size_t pointsCount = mesh.positions.size();


			mesh.positions.insert(mesh.positions.end(), faceMesh.positions.begin(), faceMesh.positions.end());
			mesh.normals.insert(mesh.normals.end(), faceMesh.normals.begin(), faceMesh.normals.end());

			assert(mesh.positions.size() == mesh.normals.size());
			auto ShiftIndex = std::bind(std::plus(), pointsCount, std::placeholders::_1);
			std::ranges::transform(faceMesh.indexes, faceMesh.indexes.begin(), ShiftIndex);
			mesh.indexes.insert(mesh.indexes.end(), faceMesh.indexes.begin(), faceMesh.indexes.end());
		}
	}

	return mesh;
}

std::pair<math::Plane, double> calcPrintPlaneAndHeight(const Mesh& mesh, const glm::vec3& direction)
{
	auto toShift = [normDir = glm::normalize(direction)](const glm::vec3& vec)
		{
			return -glm::dot(vec, normDir);
		};

	const auto [min, max] = std::ranges::minmax_element(mesh.positions, {}, toShift);
	if (min == mesh.positions.end() || max == mesh.positions.end())
		throw std::logic_error(""); // TODO

	const math::Plane printPlane(direction, *min);
	return std::make_pair(printPlane, math::distance(*max, printPlane));
}

OrientationStatByMesh calcOrientationStatByMesh(kapi::ksBodyPtr body, double overhangThreshold, double offsetThreshold)
{
	OrientationStatByMesh result;
	result.evalMesh = generateIcosphere();
	result.infos = calcOrientationsEstimation(copyToMesh(body), result.evalMesh.normals, overhangThreshold, offsetThreshold);
	result.complexInfos = calcOrientationsComplexEstimation(result.infos);
	return result;
}

std::vector<size_t> OrientationStatByMesh::findBest(OrientationComplexCriteria criteria, size_t count) const
{
	const auto& complexEstimation = complexInfos[enums::toUnderlying(criteria)];
	std::vector<size_t> indexes(complexEstimation.size());
	std::iota(indexes.begin(), indexes.end(), 0);

	auto indexToElem = [&complexEstimation, criteria](size_t index)
	{
		return complexEstimation[index];
	};
	std::ranges::partial_sort(indexes, indexes.begin() + count, {}, indexToElem);

	return std::vector<size_t>(indexes.begin(), indexes.begin() + count);
}
