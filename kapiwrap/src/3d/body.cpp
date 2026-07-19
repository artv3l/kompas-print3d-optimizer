#include "kapiwrap/3d/body.hpp"

#include <eigen3/Eigen/Dense>

geom3d::Gabarit getGabarit(kapi::ksBodyPtr body)
{
	Eigen::Vector3d begin, end;
	body->GetGabarit(&begin.x(), &begin.y(), &begin.z(), &end.x(), &end.y(), &end.z());
	return geom3d::Gabarit(begin, end);
}

ksapi::IEvolutionPtr createEvolution(ksapi::IPartPtr part, const Sketch& sketch, ksEvolutionShiftSketchTypeEnum sketchShiftType, std::vector<ksapi::IModelObjectPtr> edges)
{
	ksapi::IModelContainerPtr modelCont = part;
	ksapi::IEvolutionsPtr evolutions = modelCont->GetEvolutions();

	ksapi::IEvolutionPtr evolution = evolutions->Add(ksObj3dTypeEnum::o3d_bossEvolution);
	evolution->SetSketch(sketch.getObject());
	evolution->SetSketchShiftType(ksEvolutionShiftSketchTypeEnum::ksEvShiftKeepAngle);
	evolution->SetEdges(edges);

	evolution->Update();
	return evolution;
}
