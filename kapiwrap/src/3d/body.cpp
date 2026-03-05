#include "3d/body.hpp"

geometry::Gabarit3D getGabarit(kapi::ksBodyPtr body)
{
	geometry::Gabarit3D gabarit;
	body->GetGabarit(&gabarit.x.begin, &gabarit.y.begin, &gabarit.z.begin, &gabarit.x.end, &gabarit.y.end, &gabarit.z.end);
	return gabarit;
}
