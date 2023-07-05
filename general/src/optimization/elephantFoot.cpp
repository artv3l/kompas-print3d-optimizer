#include "stdafx.h"
#include "optimization/elephantFoot.hpp"

#include <list>
#include <utility>
#include <sstream>

#include "settings/PrintSurface.hpp"
#include "apiutil/Macro.hpp"
#include "settings/DocumentData.hpp"
#include "settings/Setting.hpp"

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

ksEntityPtr createElephantFootChamfers(ksPartPtr part, std::list<ksLoopPtr> elephantFootTargets, DocumentData::Settings& settings) {
	Macro macro(part, MACRO_NAME_ELEPHANT_FOOT, true);
	for (ksLoopPtr loopTarget : elephantFootTargets) {
		ksEntityPtr chamferEntity(part->NewEntity(Obj3dType::o3d_chamfer));
		ksChamferDefinitionPtr chamfer(chamferEntity->GetDefinition());

		double width = settings.getNumericSetting(SI_ELEPHANT_FOOT_LAYERS_COUNT.name)->getValue() * settings.getNumericSetting(SI_LAYER_HEIGHT.name)->getValue();
		chamfer->SetChamferParam(true, width, width);
		ksEntityCollectionPtr array(chamfer->array());

		ksEdgeCollectionPtr edges(loopTarget->EdgeCollection());
		int nEdges = edges->GetCount();
		for (int iEdge = 0; iEdge < nEdges; iEdge++) {
			array->Add(edges->GetByIndex(iEdge));
		}

		if (chamferEntity->Create()) {
			{ // Привязываем размеры к переменным
				ksFeaturePtr feature(chamferEntity->GetFeature());
				ksVariableCollectionPtr variableCollection(feature->VariableCollection);
				ksVariablePtr variable2(variableCollection->GetByIndex(2)); // Индекс=2 - "Длина 1"
				ksVariablePtr variable3(variableCollection->GetByIndex(3)); // Индекс=3 - "Длина 2"

				std::ostringstream oss;
				oss << settings.getNumericSetting(SI_ELEPHANT_FOOT_LAYERS_COUNT.name)->getExpression() << " * " << settings.getNumericSetting(SI_LAYER_HEIGHT.name)->getExpression();
				variable2->Expression = oss.str().c_str();
				variable3->Expression = oss.str().c_str();
			}

			macro.add(chamferEntity);
		}
	}
	return macro.getEntity();
}

std::pair<size_t, ksEntityPtr> optimizeElephantFoot(ksPartPtr part, DocumentData::Settings& settings) {
	std::list<ksLoopPtr> targets = getElephantFootTargets(part, settings.getPrintSurface());
	if (targets.empty()) {
		return std::make_pair(0, nullptr);
	}
	return std::make_pair(
		targets.size(),
		createElephantFootChamfers(part, targets, settings)
	);
}
