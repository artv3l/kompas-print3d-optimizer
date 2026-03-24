#pragma once

#include <algorithm>
#include <array>
#include <cmath>

#include <glm/glm.hpp>

namespace math
{
class Triangle final
{
public:
	static constexpr size_t c_numberOfPoints = 3;

	Triangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

	double area() const;

	std::array<glm::vec3, c_numberOfPoints> points;
};

class Plane final
{
public:
	Plane(const glm::vec3& normal, const glm::vec3& position);
	Plane(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
	Plane(const Triangle& triangle);

	glm::vec3 getNormal() const;
	float getDistance() const;
private:
	glm::vec3 m_normal;
	float m_distance;
};

class Placement final
{
public:
	glm::vec3 m_origin;
	glm::vec3 m_axisX;
	glm::vec3 m_axisY;
	glm::vec3 m_axisZ;

	Placement(const glm::vec3& origin, const glm::vec3& axisX, const glm::vec3& axisY, const glm::vec3& axisZ);

	// Матрица перехода из локальной системы координат плейсмента в глобальную (мировую)
	glm::mat4 matrixToWorld() const;

	static Placement createByAxisZ(const glm::vec3& origin, const glm::vec3& axisZ);
};

glm::vec3 project(const glm::vec3& a, const glm::vec3& b);
glm::vec3 project(const glm::vec3& vec, const Plane& plane);
Triangle project(const Triangle& triangle, const Plane& plane);
double distance(const glm::dvec3& point, const Plane& plane);
bool equal(double a, double b, double epsilon = 0.00001);
double toRadians(double angleInDegrees);
// Получить острый угол от 0 до pi/2
double toAcuteAngle(double angleInRadians);
// Привести значение baseValue из промежутка [baseBegin, baseBegin + baseLength] в промежуток [resultBegin, resultBegin + resultLength]
double convertRanges(double baseValue, double baseBegin, double baseLength, double resultBegin, double resultLength);
// Получить матрицу перехода из мировых координат в локальные координаты placement
glm::mat4 worldToLocal(const math::Placement& placement);
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

// TODO Все функции ниже перенести в kompas-print3d-optimizer / PrintSurface.hpp. Находится тут т.к. тесты пока подключены только к generic
// Находится ли треугольник на плоскости печати с учетом погрешности
bool isOnPrintPlane(const math::Triangle& triangle, const math::Plane& printPlane, double offsetThreshold);
// Рассчитать объем нависания (объем  между треугольником overhang и его проекцией на плоскость printPlane)
double volumeUnderOverhang(const math::Plane& printPlane, const math::Triangle& overhang);
