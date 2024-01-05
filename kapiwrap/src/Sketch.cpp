#include "Sketch.hpp"

Sketch::Sketch(KompasObjectPtr kompas, ksPartPtr part, IDispatchPtr plane) :
	entity(part->NewEntity(o3d_sketch)), definition(entity->GetDefinition()) {
	definition->SetPlane(plane);
	entity->Create();
	document2d = definition->BeginEdit();
	document2d_api7 = kompas->TransferInterface(document2d, ksAPI7Dual, 0);

	IViewsAndLayersManagerPtr viewsAndLayersManager(document2d_api7->ViewsAndLayersManager);
	IViewsPtr views(viewsAndLayersManager->Views);
	view = IViewPtr(views->ActiveView);
	drawingContainer = IDrawingContainerPtr(view);
}

void Sketch::endEdit() const {
	definition->EndEdit();
}
