#pragma once

#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>

namespace geom3d
{
using Triangle = std::array<Eigen::Vector3d, 3>;
using Plane = Eigen::Hyperplane<double, 3>;
using Vec3 = Eigen::Vector3d;
using Gabarit = Eigen::AlignedBox3d;

double triangleArea(const geom3d::Triangle& triangle);
// РЈРіРѕР» РјРµР¶РґСѓ РІРµРєС‚РѕСЂР°РјРё РІ СЂР°РґРёР°РЅР°С…
double angleBetween(const Vec3& a, const Vec3& b);

class Placement final
{
public:
	Placement(const Vec3& origin, const Vec3& axisX, const Vec3& axisY, const Vec3& axisZ);

	// РњР°С‚СЂРёС†Р° РїРµСЂРµС…РѕРґР° РёР· Р»РѕРєР°Р»СЊРЅРѕР№ СЃРёСЃС‚РµРјС‹ РєРѕРѕСЂРґРёРЅР°С‚ РїР»РµР№СЃРјРµРЅС‚Р° РІ РіР»РѕР±Р°Р»СЊРЅСѓСЋ (РјРёСЂРѕРІСѓСЋ)
	Eigen::Affine3d matrixToWorld() const;
	// РњР°С‚СЂРёС†Р° РїРµСЂРµС…РѕРґР° РёР· РіР»РѕР±Р°Р»СЊРЅРѕР№ (РјРёСЂРѕРІРѕР№) СЃРёСЃС‚РµРјС‹ РєРѕРѕСЂРґРёРЅР°С‚ РІ Р»РѕРєР°Р»СЊРЅСѓСЋ РЎРљ РїР»РµР№СЃРјРµРЅС‚Р°
	Eigen::Affine3d matrixToPlacement() const;

	static Placement createByAxisZ(const Vec3& origin, const Vec3& axisZ);
	static Placement createDefault();

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

// Р Р°СЃСЃС‡РёС‚Р°С‚СЊ РіР°Р±Р°СЂРёС‚ СЃРµС‚РєРё РІ Р»РѕРєР°Р»СЊРЅС‹С… РєРѕРѕСЂРґРёРЅР°С‚Р°С… РїР»РµР№СЃРјРµРЅС‚Р°
Gabarit calcGabarit(const Mesh& mesh, const Placement& placement);
}
