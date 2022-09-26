#include "stdafx.h"
#include "optimizeElephantFoot.hpp"

#include <iostream>
#include <set>

#include "selectPlane.hpp"
#include "Macro.hpp"

const char* MACRO_NAME_ELEPHANT_FOOT = "Оптимизирующие фаски слоновьей ноги";

void optimizeElephantFoot(KompasObjectPtr kompas, ksFaceDefinitionPtr face, PlaneEq planeEq, double width) {
	IApplicationPtr api7 = kompas->ksGetApplication7();
	IKompasDocument3DPtr document3d(api7->GetActiveDocument());
	IPart7Ptr topPart(document3d->GetTopPart());
	ksPartPtr part = kompas->TransferInterface(topPart, 1, 0);
	ksFeaturePtr feature(part->GetFeature());
	Macro macro(part, MACRO_NAME_ELEPHANT_FOOT, true);

	ksEntityCollectionPtr entityCollection(feature->EntityCollection(o3d_face));
	std::set<ksEdgeDefinitionPtr> edgesTargets;
	for (int i = 0; i < entityCollection->GetCount(); i++) {
		ksEntityPtr entity(entityCollection->GetByIndex(i));
		ksFaceDefinitionPtr currFace(entity->GetDefinition());
		if (currFace && currFace->IsPlanar()) {
			PlaneEq currPlaneEq(currFace);
			if (currPlaneEq.equals(planeEq)) {
				std::cout << "Найдена поверхность в плоскости печати \n";
				ksEdgeCollectionPtr edges(currFace->EdgeCollection());
				for (int i = 0; i < edges->GetCount(); i++) {
					edgesTargets.insert(ksEdgeDefinitionPtr(edges->GetByIndex(i)));
				}
			}
		}
	}
	for (std::set<ksEdgeDefinitionPtr>::iterator iter = edgesTargets.begin(); iter != edgesTargets.end(); iter++) {
		ksEntityCollectionPtr allEdgesCollection(feature->EntityCollection(o3d_edge));
		if (allEdgesCollection->FindIt((*iter)->GetEntity()) != -1) {
			ksEntityPtr chamferEntity(part->NewEntity(o3d_chamfer));
			ksChamferDefinitionPtr chamfer(chamferEntity->GetDefinition());
			ksEntityCollectionPtr array(chamfer->array());
			chamfer->SetChamferParam(true, width, width);
			array->Add((*iter));
			chamferEntity->hidden = true;
			bool isCreated = chamferEntity->Create();
			if (isCreated) {
				macro.add(chamferEntity);
			} else {
				array->Clear();
			}
		}
	}
	document3d->RebuildDocument();
}
