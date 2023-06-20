#include "stdafx.h"
#include "optimization/rounding.hpp"

#include <list>
#define _USE_MATH_DEFINES \ #include <cmath>

#include "apiutil/Macro.hpp"
#include "utils.hpp"

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

void roundEdges(ksPartPtr part, std::list<ksEdgeDefinitionPtr> roundingTargets, double radius) {
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
}

void optimizeRounding(ksPartPtr part, ksFaceDefinitionPtr printFace, const Settings& settings) {
	std::list<ksEdgeDefinitionPtr> roundingTargets = getRoundingTargets(part, printFace, settings.roundingDeflectionAngle);
	roundEdges(part, roundingTargets, settings.roundingRadius);
}
