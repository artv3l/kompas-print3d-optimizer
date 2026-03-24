#include "geometry2d.hpp"

namespace geometry
{
Polygon::Polygon(std::vector<Vector2D> points):
    m_points(points)
{}

Polygon::Polygon(size_t count, const Vector2D& initValue):
	m_points(count, initValue)
{}

void Polygon::setPoint(size_t index, const Vector2D& point)
{
	m_points[index] = point;
}

double Polygon::area() const
{
	if (m_points.size() < 3) {
		return 0.0;
	}

    double sum = 0.0;
    for (size_t i = 0; i < m_points.size(); ++i)
    {
        const auto& a = m_points[i];
        const auto& b = (i != m_points.size() - 1) ? m_points[i + 1] : m_points[0];
        sum += a.x * b.y - b.x * a.y;
    }
    return std::abs(sum) / 2.0;
}
}
