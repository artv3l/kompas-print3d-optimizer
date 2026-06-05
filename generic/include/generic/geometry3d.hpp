#pragma once

#include <eigen3/Eigen/Dense>

namespace geom3d
{
// Axis-Aligned Bounding Box
class Gabarit final
{
public:
	Gabarit(Eigen::Vector3d begin, Eigen::Vector3d end);

	Eigen::Vector3d center() const;
	Eigen::Vector3d getBegin() const;
	Eigen::Vector3d getEnd() const;

private:
	Eigen::Vector3d m_begin;
	Eigen::Vector3d m_end;
};
}
