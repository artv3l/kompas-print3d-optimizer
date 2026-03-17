#include "math.hpp"

#include <algorithm>
#include <ranges>
#include <numbers>
#include <array>

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

Plane::Plane(const Triangle& triangle):
	Plane(triangle.points[0], triangle.points[1], triangle.points[2])
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

double Triangle::area() const
{
	auto ab = points[1] - points[0];
	auto ac = points[2] - points[0];
	return glm::length(glm::cross(ab, ac)) / 2.0;
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

Triangle project(const Triangle& triangle, const Plane& plane)
{
	auto projectPoint = [&plane, &triangle](size_t i)
	{
		return project(triangle.points[i], plane);
	};
	return Triangle(projectPoint(0), projectPoint(1), projectPoint(2));
}

double distance(const glm::dvec3& point, const Plane& plane)
{
	Plane pointPlane(plane.getNormal(), point);
	return std::abs(plane.getDistance() - pointPlane.getDistance());
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

bool isOnPrintPlane(const math::Triangle& triangle, const math::Plane& printPlane, double offsetThreshold)
{
	auto isDistLessThreshold = [&printPlane, offsetThreshold](const glm::vec3 & point) {
		return std::abs(math::distance(point, printPlane)) < offsetThreshold;
	};
	return std::ranges::all_of(triangle.points, isDistLessThreshold);
}

double volumeUnderOverhang(const math::Plane& printPlane, const math::Triangle& overhang_)
{
	math::Triangle overhang(overhang_);
	const math::Triangle base = math::project(overhang, printPlane);

	auto toDistance = std::bind(math::distance, std::placeholders::_1, printPlane);
	std::ranges::sort(overhang.points, std::less(), toDistance);

	std::array<double, math::Triangle::c_numberOfPoints> distances;
	std::ranges::transform(overhang.points, distances.begin(), toDistance);
	const double min = distances[0], middle = distances[1], max = distances[2];

	const double baseVolume = base.area() * (min);

	if (math::equal(min, max))
		return baseVolume;
	else {
		const math::Plane basePlane(printPlane.getNormal(), overhang.points[0]);
		const math::Triangle pyramidBase(overhang.points[1], overhang.points[2], math::project(overhang.points[2], basePlane));

		double pyramidBaseArea = pyramidBase.area();
		if (!math::equal(min, middle)) {
			const math::Triangle pyramidBase2(
				math::project(overhang.points[1], basePlane),
				overhang.points[1],
				math::project(overhang.points[2], basePlane)
			);
			pyramidBaseArea += pyramidBase2.area();
		}

		const double topVolume = pyramidBaseArea * math::distance(overhang.points[0], math::Plane(pyramidBase)) / 3.0;
		return baseVolume + topVolume;
	}
}
