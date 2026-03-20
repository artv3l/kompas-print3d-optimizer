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

OrientationsEstimation calcOrientationsEstimation(const Mesh& mesh, std::span<const glm::vec3> directions, double overhangThreshold, double offsetThreshold)
{
	assert(mesh.indexes.size() % 3 == 0);

	const double overhangThresholdRad = degreeToRadian(overhangThreshold);

	OrientationsEstimation result;
	std::ranges::for_each(result, [&directions](std::vector<double>& vector) { vector.resize(directions.size(), 0.0); });

	for (size_t i = 0; i < directions.size(); ++i) {
		const auto& direction = directions[i];
		const auto [printPlane, height] = calcPrintPlaneAndHeight(mesh, direction);

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
				result[enums::toUnderlying(OrientationCriteria::bottomArea)][i] += triangleArea;
			} else if (angleRad < overhangThresholdRad) {
				// Площадь под мостами тоже считаем за площадь нависаний
				result[enums::toUnderlying(OrientationCriteria::overhangArea)][i] += triangleArea;
				result[enums::toUnderlying(OrientationCriteria::overhangVolume)][i] += volumeUnderOverhang(printPlane, triangle);
			}

			// TODO Convex hull
		}

		result[enums::toUnderlying(OrientationCriteria::modelHeight)][i] = height;
	}

	return result;
}

namespace
{
// Преобразовать абсолюные значения в относительные [0, 1]
std::vector<double> toRelative(std::span<const double> absoluteValues)
{
	std::vector<double> relativeValues(absoluteValues.size(), 0.0);

	const auto [min, max] = std::ranges::minmax_element(absoluteValues);
	if (min == absoluteValues.end() || max == absoluteValues.end())
		throw std::logic_error(""); // TODO

	auto convert = std::bind(math::convertRanges, std::placeholders::_1, *min, *max, 0.0, 1.0);
	std::ranges::transform(absoluteValues, relativeValues.begin(), convert);

	return relativeValues;
}
}

OrientationsComplexEstimation calcOrientationsComplexEstimation(const OrientationsEstimation& estimation)
{
	OrientationsComplexEstimation result;
	result[enums::toUnderlying(OrientationComplexCriteria::modelHeight)] = toRelative(estimation[enums::toUnderlying(OrientationCriteria::modelHeight)]);
	result[enums::toUnderlying(OrientationComplexCriteria::overhangArea)] = toRelative(estimation[enums::toUnderlying(OrientationCriteria::overhangArea)]);

	// TODO
	result[enums::toUnderlying(OrientationComplexCriteria::overhangAreaAndVolume)] = std::vector<double>(estimation[enums::toUnderlying(OrientationCriteria::overhangArea)].size(), 0.0);
	result[enums::toUnderlying(OrientationComplexCriteria::bottomQuality)] = std::vector<double>(estimation[enums::toUnderlying(OrientationCriteria::overhangArea)].size(), 0.0);
	result[enums::toUnderlying(OrientationComplexCriteria::common)] = std::vector<double>(estimation[enums::toUnderlying(OrientationCriteria::overhangArea)].size(), 0.0);

	return result;
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

OrientationStatByMesh calcOrientationStatByMesh(kapi::ksBodyPtr body, double overhangThreshold)
{
	OrientationStatByMesh result;
	result.evalMesh = generateIcosphere();
	result.estimations = calcOrientationsEstimation(copyToMesh(body), result.evalMesh.normals, overhangThreshold, 2.0);
	result.complexEstimations = calcOrientationsComplexEstimation(result.estimations);
	return result;
}

std::span<const double> OrientationStatByMesh::getByCriteria(OrientationCriteria criteria) const
{
	return estimations[enums::toUnderlying(criteria)];
}

std::vector<size_t> OrientationStatByMesh::findBest(OrientationComplexCriteria criteria, size_t count) const
{
	const auto& complexEstimation = complexEstimations[enums::toUnderlying(criteria)];
	std::vector<size_t> indexes(complexEstimation.size());
	std::iota(indexes.begin(), indexes.end(), 0);

	auto indexToElem = [&complexEstimation, criteria](size_t index)
	{
		return complexEstimation[index];
	};
	std::ranges::partial_sort(indexes, indexes.begin() + count, {}, indexToElem);

	return std::vector<size_t>(indexes.begin(), indexes.begin() + count);
}
