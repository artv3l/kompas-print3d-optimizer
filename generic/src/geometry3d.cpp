#include "geometry3d.hpp"

namespace geometry
{
double ValueRange::center() const
{
	return (begin + end) / 2.0;
}

double ValueRange::length() const
{
	return end - begin;
}

Vector3D Gabarit3D::center() const
{
	return Vector3D(x.center(), y.center(), z.center());
}
}
