#include "stdafx.h"
#include "optimization/elephantFoot.hpp"

#include <list>

#include "PrintSurface.hpp"
#include "apiutil/Macro.hpp"
#include "SettingsManager.hpp"

const char* MACRO_NAME_ELEPHANT_FOOT = "Фаски слоновьей ноги";

std::list<ksLoopPtr> getElephantFootTargets(ksPartPtr part, PrintSurface printSurface) {
	std::list<ksLoopPtr> elephantFootTargets;

	ksBodyPtr body = part->GetMainBody();
	ksFaceCollectionPtr faces = body->FaceCollection();
	int nFaces = faces->GetCount();
	for (int iFace = 0; iFace < nFaces; iFace++) {
		ksFaceDefinitionPtr face = faces->GetByIndex(iFace);
		if (!face->IsPlanar()) {
			continue;
		}
		if ((face != printSurface.face) && (PlaneEq(face) != printSurface.eq)) {
			continue;
		}
		ksLoopCollectionPtr loops(face->LoopCollection());
		for (int iLoop = 0; iLoop < loops->GetCount(); iLoop++) {
			elephantFootTargets.push_back(loops->GetByIndex(iLoop));
		}
	}
	return elephantFootTargets;
}

void createElephantFootChamfers(ksPartPtr part, std::list<ksLoopPtr> elephantFootTargets, double width) {
	Macro macro(part, MACRO_NAME_ELEPHANT_FOOT, true);
	for (ksLoopPtr loopTarget : elephantFootTargets) {
		ksEntityPtr chamferEntity(part->NewEntity(Obj3dType::o3d_chamfer));
		ksChamferDefinitionPtr chamfer(chamferEntity->GetDefinition());
		chamfer->SetChamferParam(true, width, width);
		ksEntityCollectionPtr array(chamfer->array());

		ksEdgeCollectionPtr edges(loopTarget->EdgeCollection());
		int nEdges = edges->GetCount();
		for (int iEdge = 0; iEdge < nEdges; iEdge++) {
			array->Add(edges->GetByIndex(iEdge));
		}

		if (chamferEntity->Create()) {
			macro.add(chamferEntity);
		}
	}
}

size_t optimizeElephantFoot(ksPartPtr part, const Settings& settings) {
	std::list<ksLoopPtr> targets = getElephantFootTargets(part, settings.printSurface.value());
	if (targets.empty()) {
		return 0;
	}
	createElephantFootChamfers(part, targets, settings.elephantFootLayersCount * settings.layerHeight);
	return targets.size();
}
