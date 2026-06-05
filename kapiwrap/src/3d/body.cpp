#include "3d/body.hpp"

#include <eigen3/Eigen/Dense>

geom3d::Gabarit getGabarit(kapi::ksBodyPtr body)
{
	Eigen::Vector3d begin, end;
	body->GetGabarit(&begin.x(), &begin.y(), &begin.z(), &end.x(), &end.y(), &end.z());
	return geom3d::Gabarit(begin, end);
}
