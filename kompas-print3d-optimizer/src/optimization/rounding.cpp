#include "rounding.hpp"

#include <list>

#include "utils.hpp"
#include "settings/Settings.hpp"
#include "settings/Setting.hpp"

const char* NAME_ROUNDING = "Скругления для выпирающих углов";

bool isEdgeForRounding(ksEdgeDefinitionPtr edge, ksPartPtr part, ksFaceDefinitionPtr printFace, double deflectionAngle) {
	if (!edge->IsStraight()) {
		return false;
	}
	ksFaceDefinitionPtr face1(edge->GetAdjacentFace(true)); ksFaceDefinitionPtr face2(edge->GetAdjacentFace(false));
	if (!face1 || !face2) {
		return false;
	}
	if (!face1->IsPlanar() || !face2->IsPlanar()) {
		return false;
	}
	ksMeasurerPtr measurer(part->GetMeasurer());
	measurer->SetObject1(printFace);
	measurer->SetObject2(edge);
	measurer->Calc();
	double angle = 90.0 - measurer->angle;
	if (angle > deflectionAngle) {
		return false;
	}
	return true;
}

std::list<ksEdgeDefinitionPtr> getRoundingTargets(ksPartPtr part, ksFaceDefinitionPtr printFace, double deflectionAngle) {
	ksEntityCollectionPtr entityCollection(part->EntityCollection(Obj3dType::o3d_edge));
	std::list<ksEdgeDefinitionPtr> roundingTargets;

	int nEntities = entityCollection->GetCount();
	for (int iEntity = 0; iEntity < nEntities; iEntity++) {
		ksEntityPtr entity(entityCollection->GetByIndex(iEntity));
		ksEdgeDefinitionPtr edge(entity->GetDefinition());
		if (isEdgeForRounding(edge, part, printFace, deflectionAngle)) {
			roundingTargets.push_back(edge);
		}
	}
	return roundingTargets;
}

ksEntityPtr roundEdges(ksPartPtr part, std::list<ksEdgeDefinitionPtr> roundingTargets, double radius) {
	ksEntityPtr filletEntity(part->NewEntity(o3d_fillet));
	ksFilletDefinitionPtr fillet(filletEntity->GetDefinition());
	ksEntityCollectionPtr array(fillet->array());
	filletEntity->name = NAME_ROUNDING;
	fillet->radius = radius;
	fillet->tangent = false;
	for (ksEdgeDefinitionPtr edge : roundingTargets) {
		array->Add(edge);
	}

	filletEntity->Create();
	return filletEntity;
}

ksEntityPtr optimizeRounding(ksPartPtr part, Settings& settings) {
	std::list<ksEdgeDefinitionPtr> targets =
		getRoundingTargets(part, settings.getPrintSurface().face, settings.getNumericSetting(SI_ROUNDING_DEFLECTION_ANGLE.name)->getValue());
	if (targets.empty()) {
		return nullptr;
	}
	
	ksEntityPtr filletEntity = roundEdges(part, targets, settings.getNumericSetting(SI_ROUNDING_RADIUS.name)->getValue());
	return filletEntity;
}
