#pragma once

namespace geometry
{
struct Vector3D final
{
	double x;
	double y;
	double z;
};

// begin <= end
struct ValueRange final
{
	double begin;
	double end;

	double center() const;
	double length() const;
};

struct Gabarit3D final
{
	ValueRange x;
	ValueRange y;
	ValueRange z;

	Vector3D center() const;
};
}
