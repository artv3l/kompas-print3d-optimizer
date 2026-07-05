#pragma once

#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>

namespace geom3d
{
using Triangle = std::array<Eigen::Vector3d, 3>;
using Plane = Eigen::Hyperplane<double, 3>;
using Vec3 = Eigen::Vector3d;

double triangleArea(const geom3d::Triangle& triangle);
// РЈРіРѕР» РјРµР¶РґСѓ РІРµРєС‚РѕСЂР°РјРё РІ СЂР°РґРёР°РЅР°С…
double angleBetween(const Vec3& a, const Vec3& b);

// Axis-Aligned Bounding Box
class Gabarit final
{
public:
	Gabarit(Vec3 begin, Vec3 end);

	Vec3 center() const;
	Vec3 getBegin() const;
	Vec3 getEnd() const;

private:
	Vec3 m_begin;
	Vec3 m_end;
};

class Placement final
{
public:
	Placement(const Vec3& origin, const Vec3& axisX, const Vec3& axisY, const Vec3& axisZ);

	// РњР°С‚СЂРёС†Р° РїРµСЂРµС…РѕРґР° РёР· Р»РѕРєР°Р»СЊРЅРѕР№ СЃРёСЃС‚РµРјС‹ РєРѕРѕСЂРґРёРЅР°С‚ РїР»РµР№СЃРјРµРЅС‚Р° РІ РіР»РѕР±Р°Р»СЊРЅСѓСЋ (РјРёСЂРѕРІСѓСЋ)
	Eigen::Affine3d matrixToWorld() const;

	static Placement createByAxisZ(const Vec3& origin, const Vec3& axisZ);

private:
	Vec3 m_origin;
	Vec3 m_axisX;
	Vec3 m_axisY;
	Vec3 m_axisZ;
};

class Mesh final
{
public:
	using Index = size_t;

	std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	std::vector<Index> indexes;
};
}
