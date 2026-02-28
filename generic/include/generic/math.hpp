#pragma once

#include <algorithm>
#include <array>
#include <cmath>

#include <glm/glm.hpp>

namespace math
{
class Plane final
{
public:
	Plane(const glm::vec3& normal, const glm::vec3& position);
	Plane(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

	glm::vec3 getNormal() const;
	float getDistance() const;
private:
	glm::vec3 m_normal;
	float m_distance;
};

class Triangle final
{
public:
	static constexpr size_t c_numberOfPoints = 3;

	Triangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

	std::array<glm::vec3, c_numberOfPoints> points;
};

glm::vec3 project(const glm::vec3& a, const glm::vec3& b);
glm::vec3 project(const glm::vec3& vec, const Plane& plane);
double distance(const glm::dvec3& point, const Plane& plane);
bool equal(double a, double b, double epsilon = 0.00001);
double toRadians(double angleInDegrees);
// Получить острый угол от 0 до pi/2
double toAcuteAngle(double angleInRadians);
}

template <typename T>
typename T::value_type calcTriangleArea(const T& a, const T& b, const T& c)
{
	auto ab = b - a;
	auto ac = c - a;
	return glm::length(glm::cross(ab, ac)) / static_cast<T::value_type>(2.0);
}

// Угол между векторами в радианах
template <typename T>
typename T::value_type calcAngleBetween(const T& a, const T& b)
{
	auto dot = glm::dot(a, b);
	auto len = glm::length(a) * glm::length(b);
	if (math::equal(len, 0.0))
		return 0.0;
	return std::acos(std::clamp(dot / len, T::value_type(-1), T::value_type(1)));
}

// TODO Перенести в kompas-print3d-optimizer / PrintSurface.hpp. Находится тут т.к. тесты пока подключены только к generic
// Находится ли треугольник на плоскости печати с учетом погрешностей
bool isOnPrintPlane(const math::Triangle& triangle, const math::Plane& printPlane, double angleThreshold, double offsetThreshold);
