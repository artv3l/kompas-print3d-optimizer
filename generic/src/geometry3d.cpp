#include "geometry3d.hpp"

#include "math.hpp"

namespace geom3d
{
double triangleArea(const geom3d::Triangle& triangle)
{
	auto ab = triangle[1] - triangle[0];
	auto ac = triangle[2] - triangle[0];
	return ab.cross(ac).norm() / 2.0;
}

double angleBetween(const Vec3& a, const Vec3& b)
{
	double dot = a.dot(b);
	double len = a.norm() * b.norm();
	if (math::equal(len, 0.0))
		return 0.0;
	return std::acos(std::clamp(dot / len, -1.0, 1.0));
}

Placement::Placement(const Vec3& origin, const Vec3& axisX, const Vec3& axisY, const Vec3& axisZ) :
	m_origin(origin),
	m_axisX(axisX),
	m_axisY(axisY),
	m_axisZ(axisZ)
{
}

Eigen::Affine3d Placement::matrixToWorld() const
{
	Eigen::Affine3d placementMat = Eigen::Affine3d::Identity();
	placementMat.linear().col(0) = m_axisX;
	placementMat.linear().col(1) = m_axisY;
	placementMat.linear().col(2) = m_axisZ;
	placementMat.translation() = m_origin;
	return placementMat;
}

Eigen::Affine3d Placement::matrixToPlacement() const
{
	return matrixToWorld().inverse();
}

Placement Placement::createByAxisZ(const Vec3& origin, const Vec3& axisZ)
{
	const Vec3 c_axisX(1.0, 0.0, 0.0);
	const Vec3 c_axisY(0.0, 1.0, 0.0);

	const bool isUseAxisX = math::toAcuteAngle(angleBetween(axisZ, c_axisX)) > math::toRadians(10);

	const Vec3 axisX = isUseAxisX ? axisZ.cross(c_axisX) : axisZ.cross(c_axisY);
	const Vec3 axisY = axisX.cross(axisZ);

	return Placement(origin, axisX.normalized(), axisY.normalized(), axisZ.normalized());
}

Placement Placement::createDefault()
{
	return Placement(Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(0, 1, 0), Vec3(0, 0, 1));
}

Gabarit calcGabarit(const Mesh& mesh, const Placement& placement)
{
	auto toLocal = placement.matrixToPlacement();

	Gabarit gabarit;
	for (const auto& pos : mesh.positions)
		gabarit.extend(toLocal * pos);
	return gabarit;
}
}
