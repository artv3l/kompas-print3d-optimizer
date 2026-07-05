#include "math.hpp"

#include <algorithm>
#include <ranges>
#include <numbers>
#include <array>

namespace math
{
bool equal(double a, double b, double epsilon)
{
	return (std::abs(a - b) < epsilon);
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

double convertRanges(double baseValue, double baseBegin, double baseLength, double resultBegin, double resultLength)
{
	const double k = resultLength / baseLength;
	const double basePos = baseValue - baseBegin;
	return resultBegin + (basePos * k);
}

double polygonArea(std::span<Eigen::Vector2d> points)
{
	if (points.size() < 3) {
		assert(false);
		return 0.0;
	}

	double sum = 0.0;
	for (size_t i = 0; i < points.size(); ++i)
	{
		const auto& a = points[i];
		const auto& b = (i != points.size() - 1) ? points[i + 1] : points[0];
		sum += a.x() * b.y() - b.x() * a.y();
	}
	return std::abs(sum) / 2.0;
}
}
