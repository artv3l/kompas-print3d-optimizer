#pragma once

#include <vector>

namespace geometry
{
struct Vector2D final
{
	double x;
	double y;
};

// РџРѕР»РёРіРѕРЅ Р±РµР· СЃР°РјРѕРїРµСЂРµСЃРµС‡РµРЅРёР№
class Polygon final
{
public:
	Polygon(std::vector<Vector2D> points);
	Polygon(size_t count, const Vector2D& initValue);

	void setPoint(size_t index, const Vector2D& point);
	double area() const;

	std::vector<Vector2D> m_points;
};
}
