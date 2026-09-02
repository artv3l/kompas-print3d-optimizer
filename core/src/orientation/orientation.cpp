#include "orientation/orientation.hpp"

#include <iostream>
#include <ranges>
#include <numeric>

#include "generic/math.hpp"
#include "convexHull.hpp"
#include "generic/perfomance.hpp"
#include "mesh.hpp"

// РќР°Р№С‚Рё РїР»РѕСЃРєРѕСЃС‚СЊ РїРµС‡Р°С‚Рё Рё РІС‹СЃРѕС‚Сѓ РјРѕРґРµР»Рё
std::pair<geom3d::Plane, double> calcPrintPlaneAndHeight(const geom3d::Mesh& mesh, const geom3d::Vec3& direction)
{
	auto toShift = [normDir = direction.normalized()](const geom3d::Vec3& vec)
		{
			return -vec.dot(normDir);
		};

	const auto [min, max] = std::ranges::minmax_element(mesh.positions, {}, toShift);
	if (min == mesh.positions.end() || max == mesh.positions.end())
		throw std::logic_error(""); // TODO

	const geom3d::Plane printPlane = geom3d::Plane(direction, *min);
	return std::make_pair(printPlane, printPlane.absDistance(*max));
}

OrientationInfo calcOrientationInfo(const geom3d::Mesh& mesh, const geom3d::Vec3& direction, double overhangThreshold, double offsetThreshold)
{
	OrientationInfo info;

	const double overhangThresholdRad = math::toRadians(overhangThreshold);
	const auto [printPlane, height] = calcPrintPlaneAndHeight(mesh, direction);

	const geom3d::Placement printPlanePlacement = geom3d::Placement::createByAxisZ(
		printPlane.projection(geom3d::Vec3(0.0, 0.0, 0.0)), printPlane.normal()
	);
	const Eigen::Affine3d toWorld = printPlanePlacement.matrixToWorld();
	const Eigen::Affine3d toPrintPlanePlacement = toWorld.inverse();

	std::vector<Eigen::Vector2d> convexHullPoints;
	convexHullPoints.reserve(mesh.positions.size());

	info.triangleProperties.resize(mesh.indexes.size() / 3);

	for (int iIndex = 0; (iIndex + 2) < mesh.indexes.size(); iIndex += 3) {
		const size_t i1 = mesh.indexes[iIndex];
		const size_t i2 = mesh.indexes[iIndex + 1];
		const size_t i3 = mesh.indexes[iIndex + 2];
		const geom3d::Triangle triangle = { mesh.positions[i1], mesh.positions[i2], mesh.positions[i3] };

		// Р”Р»СЏ РІСЃРµС… С‚СЂРµС… С‚РѕС‡РµРє Р±СѓРґСѓС‚ РѕРґРёРЅР°РєРѕРІС‹Рµ РЅРѕСЂРјР°Р»Рё, РїРѕСЌС‚РѕРјСѓ Р±РµСЂРµРј Р»СЋР±СѓСЋ (РїРµСЂРІСѓСЋ)
		const geom3d::Vec3 normal = mesh.normals[i1];
		const double angleRad = geom3d::angleBetween(normal, geom3d::Vec3(printPlane.normal()));
		const double triangleArea = geom3d::triangleArea(triangle);

		if (isOnPrintPlane(triangle, printPlane, offsetThreshold)) {
			info.bottomArea += triangleArea;
			info.triangleProperties[iIndex / 3] = TriangleProperties::bottom;
		}
		else if (angleRad < overhangThresholdRad) {
			// РџР»РѕС‰Р°РґСЊ РїРѕРґ РјРѕСЃС‚Р°РјРё С‚РѕР¶Рµ СЃС‡РёС‚Р°РµРј Р·Р° РїР»РѕС‰Р°РґСЊ РЅР°РІРёСЃР°РЅРёР№
			info.overhangArea += triangleArea;
			info.triangleProperties[iIndex / 3] = TriangleProperties::overhang;
		}

		for (auto&& pnt : triangle) {
			const geom3d::Vec3 pntLocal = toPrintPlanePlacement * pnt;
			if (std::abs(pntLocal.z()) < offsetThreshold) {
				convexHullPoints.emplace_back(pntLocal.x(), pntLocal.y());
			}
		}
	}

	info.modelHeight = height;

	if (convexHullPoints.size() >= 3) {
		std::vector<Eigen::Vector2d> hullPolygon = convexHull(convexHullPoints);
		info.bottomConvexHullArea = (hullPolygon.size() >= 3) ? math::polygonArea(hullPolygon) : 0.0;

		for (size_t i = 0; i < hullPolygon.size(); ++i) {
			const auto& point = hullPolygon[i];
			info.bottomContour.push_back(toWorld * geom3d::Vec3(point.x(), point.y(), 0.0f));
		}
	}
	else {
		for (auto&& pnt : convexHullPoints) {
			info.bottomContour.push_back(toWorld * geom3d::Vec3(pnt.x(), pnt.y(), 0.0f));
		}
	}

	return info;
}

// РџСЂРµРѕР±СЂР°Р·РѕРІР°С‚СЊ Р°Р±СЃРѕР»СЋРЅС‹Рµ Р·РЅР°С‡РµРЅРёСЏ РІ РѕС‚РЅРѕСЃРёС‚РµР»СЊРЅС‹Рµ [0, 1]
template <std::ranges::range R>
std::vector<double> toRelative(R absoluteValues)
{
	std::vector<double> relativeValues(absoluteValues.size(), 0.0);

	const auto [min, max] = std::ranges::minmax_element(absoluteValues);
	if (min == absoluteValues.end() || max == absoluteValues.end())
		throw std::logic_error(""); // TODO

	auto convert = std::bind(math::convertRanges, std::placeholders::_1, *min, *max, 0.0, 1.0);
	std::ranges::transform(absoluteValues, relativeValues.begin(), convert);

	return relativeValues;
}

// Р Р°СЃСЃС‡РёС‚Р°С‚СЊ РІСЃРµ СЃРѕСЃС‚Р°РІРЅС‹Рµ РєСЂРёС‚РµСЂРёРё
OrientationComplexInfos calcOrientationsComplexEstimation(std::span<OrientationInfo> infos)
{
	auto perfLock = perfomance::measureTime([](std::chrono::nanoseconds time) {
		std::cout << "Composite criteria calc: "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(time)
			<< "\n";
		});

	size_t size = infos.size();

	OrientationComplexInfos result;
	std::ranges::for_each(result, [size](auto& vector) { vector.resize(size); });

	auto& overhangs = result[enums::toUnderlying(OrientationComplexCriteria::overhangs)];
	{
		auto relOverhangAreas = toRelative(infos | std::views::transform(&OrientationInfo::overhangArea));
		overhangs = relOverhangAreas;
	}
	
	auto& bottomQuality = result[enums::toUnderlying(OrientationComplexCriteria::bottomQuality)];
	{
		auto relBottomAreas = toRelative(infos | std::views::transform(&OrientationInfo::bottomArea));
		auto relBottomConvexHullAreas = toRelative(infos | std::views::transform(&OrientationInfo::bottomConvexHullArea));
		for (size_t i = 0; i < size; ++i) {
			bottomQuality[i] = (0.6 * (1.0 - relBottomAreas[i])) + (0.4 * (1.0 - relBottomConvexHullAreas[i]));
		}
	}

	auto& common = result[enums::toUnderlying(OrientationComplexCriteria::common)];
	{
		auto relModelHeight = toRelative(infos | std::views::transform(&OrientationInfo::modelHeight));
		for (size_t i = 0; i < size; ++i) {
			common[i] = (0.4 * overhangs[i]) + (0.4 * bottomQuality[i]) + (0.2 * relModelHeight[i]);
		}
	}

	return result;
}

// Р Р°СЃСЃС‡РёС‚Р°С‚СЊ РІСЃРµ РєСЂРёС‚РµСЂРёРё РґР»СЏ РЅРµСЃРєРѕР»СЊРєРёС… РІР°СЂРёР°РЅС‚РѕРІ РѕСЂРёРµРЅС‚Р°С†РёРё
std::vector<OrientationInfo> calcOrientationsEstimation(const geom3d::Mesh& mesh, std::span<const geom3d::Vec3> directions, double overhangThreshold, double offsetThreshold)
{
	assert(mesh.indexes.size() % 3 == 0);

	auto perfLock = perfomance::measureTime([](std::chrono::nanoseconds time) {
		std::cout << "Simple criteria calc: "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(time)
			<< "\n";
		});

	std::vector<OrientationInfo> result;
	result.resize(directions.size());
	for (size_t i = 0; i < directions.size(); ++i) {
		result[i] = calcOrientationInfo(mesh, directions[i], overhangThreshold, offsetThreshold);
	}
	return result;
}

OrientationStatByMesh calcOrientationStatByMesh(const geom3d::Mesh & mesh, double overhangThreshold, double offsetThreshold, uint8_t subdivisionsCount)
{
	OrientationStatByMesh result;

	result.model = mesh;
	result.colors = std::vector<color::RGB>(result.model.indexes.size() / 3, orientation::c_defaultColor);

	result.evalMesh = generateIcosphere(subdivisionsCount);
	result.infos = calcOrientationsEstimation(result.model, result.evalMesh.normals, overhangThreshold, offsetThreshold);
	result.complexInfos = calcOrientationsComplexEstimation(result.infos);
	return result;
}

bool isOnPrintPlane(const geom3d::Triangle& triangle, const geom3d::Plane& printPlane, double offsetThreshold)
{
	auto isDistLessThreshold = [&printPlane, offsetThreshold](const Eigen::Vector3d& point) {
		return printPlane.absDistance(point) < offsetThreshold;
		};
	return std::ranges::all_of(triangle, isDistLessThreshold);
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
	std::ranges::stable_sort(indexes, {}, indexToElem);

	return std::vector<size_t>(indexes.begin(), indexes.begin() + count);
}

void OrientationStatByMesh::updateMeshColors(size_t index)
{
	const auto& props = infos[index].triangleProperties;
	assert(props.size() == (model.indexes.size() / 3));

	for (size_t i = 0; i < props.size(); ++i) {
		color::RGB color = orientation::c_defaultColor;
		if (props[i] == TriangleProperties::overhang) {
			color = orientation::c_overhangColor;
		}
		else if (props[i] == TriangleProperties::bottom) {
			color = orientation::c_bottomColor;
		}
		colors[i] = color;
	}
}
