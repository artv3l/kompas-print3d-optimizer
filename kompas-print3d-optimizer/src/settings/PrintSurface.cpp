#include "PrintSurface.hpp"

#include <utility>
#include <stdexcept>
#include <span>
#include <algorithm>
#include <iterator>

#include <glm/glm.hpp>

#include "utils.hpp"
#include "generic/math.hpp"
#include "generic/windows.hpp"
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


// Рассчитать площадь всей сетки
double calcTotalAreaByMesh(const Mesh& mesh)
{
	assert(mesh.indexes.size() % 3 == 0);

	double totalArea = 0.0;
	for (int iIndex = 0; (iIndex + 2) < mesh.indexes.size(); iIndex += 3) {
		const size_t i1 = mesh.indexes[iIndex];
		const size_t i2 = mesh.indexes[iIndex + 1];
		const size_t i3 = mesh.indexes[iIndex + 2];
		totalArea += calcTriangleArea(mesh.positions[i1], mesh.positions[i2], mesh.positions[i3]);
	}
	return totalArea;
}

// Рассчитать суммарную площадь нависаний. overhangThreshold в градусах, direction направлено вниз от печатного стола
double calcOverhangsAreaByMesh(const Mesh & mesh, glm::vec3 direction, double overhangThreshold)
{
	assert(mesh.indexes.size() % 3 == 0);

	const double overhangThresholdRad = degreeToRadian(overhangThreshold);
	double overhangsArea = 0.0;

	for (int iIndex = 0; (iIndex + 2) < mesh.indexes.size(); iIndex += 3) {
		const size_t i1 = mesh.indexes[iIndex];
		const size_t i2 = mesh.indexes[iIndex + 1];
		const size_t i3 = mesh.indexes[iIndex + 2];

		// Для всех трех точек будут одинаковые нормали, поэтому берем любую (первую)
		const glm::vec3 normal = mesh.normals[i1];
		const double angleRad = calcAngleBetween(normal, direction);

		/*
		  Если угол равен нулю, то считаем что эта грань плоскость печати или печатается мостом.
		  Что не очень правильно, т.к. нужно считать плоскостью печати ближайшую грань в
		  направлении плоскости печати. И не учитывать только ее.

		  Пока для простоты считаем что все остальные горизонтальные грани могут напечататься мостами.
		*/
		if (!doubleEqual(angleRad, 0.0) && (angleRad < overhangThresholdRad)) {
			overhangsArea += calcTriangleArea(mesh.positions[i1], mesh.positions[i2], mesh.positions[i3]);
		}
	}

	return overhangsArea;
}

// Рассчитать суммарную площадь нависаний и площаь всего тела по тесселляции тела
std::pair<double, std::vector<double>> calcOverhangsAreaByBodyTessellation(
	kapi::ksBodyPtr body, std::span<const glm::vec3> directions, double overhangThreshold)
{
	auto faces = checkCast<kapi::ksFaceCollectionPtr>(checkPtr(body)->FaceCollection());

	std::vector<double> overhangsArea(directions.size(), 0.0);
	double bodyArea = 0.0;

	for (size_t iFace = 0, nFaces = faces->GetCount(); iFace < nFaces; ++iFace)
	{
		kapi::ksFaceDefinitionPtr face = checkPtr(faces->GetByIndex(iFace));
		kapi::ksTessellationPtr tessellation = checkPtr(face->GetTessellation());
		Mesh faceMesh = copyToMesh(tessellation);

		bodyArea += calcTotalAreaByMesh(faceMesh);

		auto calcOverhangsArea = std::bind(calcOverhangsAreaByMesh, faceMesh, std::placeholders::_1, overhangThreshold);
		std::ranges::transform(directions, overhangsArea, overhangsArea.begin(), std::plus<double>(), calcOverhangsArea);
	}

	return std::make_pair(bodyArea, overhangsArea);
}

OrientationStatByMesh calcOrientationStatByMesh(kapi::ksBodyPtr body, double overhangThreshold)
{
	Mesh icosphere = generateIcosphere();
	auto result = calcOverhangsAreaByBodyTessellation(body, icosphere.normals, overhangThreshold);

	OrientationStatByMesh stat;
	stat.body = body;
	stat.bodyArea = result.first;
	stat.evalMesh = std::move(icosphere);
	stat.overhangsArea = std::move(result.second);
	return stat;
}

Mesh copyToMesh(kapi::ksTessellationPtr tessellation)
{
	checkPtr(tessellation);

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
