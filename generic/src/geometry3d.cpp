#include "geometry3d.hpp"

namespace geom3d
{
Gabarit::Gabarit(Eigen::Vector3d begin, Eigen::Vector3d end) :
	m_begin(begin),
	m_end(end)
{
}

Eigen::Vector3d Gabarit::center() const
{
	return (m_begin + m_end) / 2.0;
}

Eigen::Vector3d Gabarit::getBegin() const
{
	return m_begin;
}

Eigen::Vector3d Gabarit::getEnd() const
{
	return m_end;
}
}
