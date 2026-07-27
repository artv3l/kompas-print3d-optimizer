#include "kapiwrap/3d/body.hpp"

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
