#include "rounding.hpp"

#include <list>

#include "utils.hpp"
#include "settings/Settings.hpp"
#include "settings/Setting.hpp"
#include "settings/SettingInitializer.hpp"

const char* NAME_ROUNDING = "Скругления для выпирающих углов";

bool isEdgeForRounding(kapi::ksEdgeDefinitionPtr edge, kapi::ksPartPtr part, kapi::ksFaceDefinitionPtr printFace, double deflectionAngle) {
	if (!edge->IsStraight()) {
		return false;
	}
	kapi::ksFaceDefinitionPtr face1(edge->GetAdjacentFace(true)); kapi::ksFaceDefinitionPtr face2(edge->GetAdjacentFace(false));
	if (!face1 || !face2) {
		return false;
	}
	if (!face1->IsPlanar() || !face2->IsPlanar()) {
		return false;
	}
	kapi::ksMeasurerPtr measurer(part->GetMeasurer());
	measurer->SetObject1(printFace);
	measurer->SetObject2(edge);
	measurer->Calc();
	double angle = 90.0 - measurer->angle;
	if (angle > deflectionAngle) {
		return false;
	}
	return true;
}

std::list<kapi::ksEdgeDefinitionPtr> getRoundingTargets(kapi::ksPartPtr part, kapi::ksFaceDefinitionPtr printFace, double deflectionAngle) {
	kapi::ksEntityCollectionPtr entityCollection(part->EntityCollection(kapi::Obj3dType::o3d_edge));
	std::list<kapi::ksEdgeDefinitionPtr> roundingTargets;

	int nEntities = entityCollection->GetCount();
	for (int iEntity = 0; iEntity < nEntities; iEntity++) {
		kapi::ksEntityPtr entity(entityCollection->GetByIndex(iEntity));
		kapi::ksEdgeDefinitionPtr edge(entity->GetDefinition());
		if (isEdgeForRounding(edge, part, printFace, deflectionAngle)) {
			roundingTargets.push_back(edge);
		}
	}
	return roundingTargets;
}

kapi::ksEntityPtr roundEdges(kapi::ksPartPtr part, std::list<kapi::ksEdgeDefinitionPtr> roundingTargets, double radius) {
	kapi::ksEntityPtr filletEntity(part->NewEntity(kapi::o3d_fillet));
	kapi::ksFilletDefinitionPtr fillet(filletEntity->GetDefinition());
	kapi::ksEntityCollectionPtr array(fillet->array());
	filletEntity->name = NAME_ROUNDING;
	fillet->radius = radius;
	fillet->tangent = false;
	for (kapi::ksEdgeDefinitionPtr edge : roundingTargets) {
		array->Add(edge);
	}

	filletEntity->Create();
	return filletEntity;
}

kapi::ksEntityPtr optimizeRounding(kapi::ksPartPtr part, Settings& settings) {
	std::list<kapi::ksEdgeDefinitionPtr> targets =
		getRoundingTargets(part, settings.getPrintSurface().face, settings.getDoubleSetting(si::roundingDeflectionAngle.name)->getValue());
	if (targets.empty()) {
		return nullptr;
	}
	
	kapi::ksEntityPtr filletEntity = roundEdges(part, targets, settings.getDoubleSetting(si::roundingRadius.name)->getValue());
	return filletEntity;
}
