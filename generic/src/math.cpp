#include "math.hpp"

#include <algorithm>
#include <numbers>

namespace math
{
Plane::Plane(const glm::vec3& normal, const glm::vec3& position):
	m_normal(glm::normalize(normal)),
	m_distance(-glm::dot(normal, position))
{
}

Plane::Plane(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c):
	m_normal(glm::normalize(glm::cross(b - a, b - c))),
	m_distance(-glm::dot(m_normal, a))
{
}

glm::vec3 Plane::getNormal() const
{
	return m_normal;
}

float Plane::getDistance() const
{
	return m_distance;
}

Triangle::Triangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) :
	points({ a, b, c })
{
}

glm::vec3 project(const glm::vec3& a, const glm::vec3& b)
{
	return (glm::dot(a, b) / glm::dot(b, b)) * b;
}

glm::vec3 project(const glm::vec3& vec, const Plane& plane)
{
	const glm::vec3 planeNormal = plane.getNormal();
	return vec - planeNormal * (glm::dot(planeNormal, vec) + plane.getDistance());
}

double distance(const glm::dvec3& point, const Plane& plane)
{
	Plane pointPlane(plane.getNormal(), point);
	return plane.getDistance() - pointPlane.getDistance();
}

bool equal(double a, double b, double epsilon)
{
	return (abs(a - b) < epsilon);
}

double toRadians(double angleInDegrees)
{
	return angleInDegrees * std::numbers::pi / 180.0;
}

double toAcuteAngle(double angleInRadians)
{
	angleInRadians = std::fmod(std::abs(angleInRadians), std::numbers::pi); // 0..pi
	return std::min(angleInRadians, std::numbers::pi - angleInRadians); // 0..pi/2
}
}

/*
  angleThreshold - Угол в радианах
*/
bool isOnPrintPlane(const math::Triangle& triangle, const math::Plane& printPlane, double angleThreshold, double offsetThreshold)
{
	auto isDistLessThreshold = [&printPlane, offsetThreshold](const glm::vec3 & point) {
		return std::abs(math::distance(point, printPlane)) < offsetThreshold;
	};
	if (!std::ranges::all_of(triangle.points, isDistLessThreshold))
		return false;

	const math::Plane trianglePlane(triangle.points[0], triangle.points[1], triangle.points[2]);
	const double angle = calcAngleBetween(trianglePlane.getNormal(), printPlane.getNormal());
	return math::toAcuteAngle(angle) < angleThreshold;
}
