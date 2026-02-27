#include "math.hpp"

namespace math
{
Plane::Plane(const glm::vec3& normal, const glm::vec3& position):
	m_normal(normal),
	m_distance(-glm::dot(normal, position))
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

glm::vec3 project(const glm::vec3& a, const glm::vec3& b)
{
	return (glm::dot(a, b) / glm::dot(b, b)) * b;
}

glm::vec3 project(const glm::vec3& vec, const Plane& plane)
{
	const glm::vec3 planeNormal = plane.getNormal();
	return vec - planeNormal * (glm::dot(planeNormal, vec) + plane.getDistance());
}
}
