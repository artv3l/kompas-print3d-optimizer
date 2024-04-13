#include "elephantFoot.hpp"

#include <list>
#include <sstream>

#include "kapiwrap/Macro.hpp"

#include "settings/PrintSurface.hpp"
#include "settings/Settings.hpp"
#include "settings/Setting.hpp"
#include "settings/SettingInitializer.hpp"

const char* MACRO_NAME_ELEPHANT_FOOT = "Фаски слоновьей ноги";

std::list<kapi::ksLoopPtr> getElephantFootTargets(kapi::ksPartPtr part, PrintSurface printSurface) {
	std::list<kapi::ksLoopPtr> elephantFootTargets;

	kapi::ksBodyPtr body = part->GetMainBody();
	kapi::ksFaceCollectionPtr faces = body->FaceCollection();
	int nFaces = faces->GetCount();
	for (int iFace = 0; iFace < nFaces; iFace++) {
		kapi::ksFaceDefinitionPtr face = faces->GetByIndex(iFace);
		if (!face->IsPlanar()) {
			continue;
		}
		if ((face != printSurface.face) && (PlaneEq(face) != printSurface.eq)) {
			continue;
		}
		kapi::ksLoopCollectionPtr loops(face->LoopCollection());
		for (int iLoop = 0; iLoop < loops->GetCount(); iLoop++) {
			elephantFootTargets.push_back(loops->GetByIndex(iLoop));
		}
	}
	return elephantFootTargets;
}

kapi::ksEntityPtr createElephantFootChamfers(kapi::ksPartPtr part, std::list<kapi::ksLoopPtr> elephantFootTargets, Settings& settings) {
	Macro macro(part, MACRO_NAME_ELEPHANT_FOOT, true);
	for (kapi::ksLoopPtr loopTarget : elephantFootTargets) {
		kapi::ksEntityPtr chamferEntity(part->NewEntity(kapi::Obj3dType::o3d_chamfer));
		kapi::ksChamferDefinitionPtr chamfer(chamferEntity->GetDefinition());

		double width = settings.getDoubleSetting(si::bridgeHoleFillLayersCount.name)->getValue() * settings.getDoubleSetting(si::layerHeight.name)->getValue();
		chamfer->SetChamferParam(true, width, width);
		kapi::ksEntityCollectionPtr array(chamfer->array());

		kapi::ksEdgeCollectionPtr edges(loopTarget->EdgeCollection());
		int nEdges = edges->GetCount();
		for (int iEdge = 0; iEdge < nEdges; iEdge++) {
			array->Add(edges->GetByIndex(iEdge));
		}

		if (chamferEntity->Create()) {
			{ // Привязываем размеры к переменным
				kapi::ksFeaturePtr feature(chamferEntity->GetFeature());
				kapi::ksVariableCollectionPtr variableCollection(feature->VariableCollection);
				kapi::ksVariablePtr variable2(variableCollection->GetByIndex(2)); // Индекс=2 - "Длина 1"
				kapi::ksVariablePtr variable3(variableCollection->GetByIndex(3)); // Индекс=3 - "Длина 2"

				std::ostringstream oss;
				oss << settings.getDoubleSetting(si::bridgeHoleFillLayersCount.name)->getExpression() << " * " << settings.getDoubleSetting(si::layerHeight.name)->getExpression();
				variable2->Expression = oss.str().c_str();
				variable3->Expression = oss.str().c_str();
			}

			macro.add(chamferEntity);
		}
	}
	return macro.getEntity();
}

kapi::ksEntityPtr optimizeElephantFoot(kapi::ksPartPtr part, Settings& settings) {
	std::list<kapi::ksLoopPtr> targets = getElephantFootTargets(part, settings.getPrintSurface());
	if (targets.empty()) {
		return nullptr;
	}
	return createElephantFootChamfers(part, targets, settings);
}
