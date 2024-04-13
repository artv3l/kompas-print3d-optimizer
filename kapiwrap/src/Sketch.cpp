#include "Sketch.hpp"

Sketch::Sketch(kapi::KompasObjectPtr kompas, kapi::ksPartPtr part, IDispatchPtr plane) :
	entity(part->NewEntity(kapi::o3d_sketch)), definition(entity->GetDefinition()) {
	definition->SetPlane(plane);
	entity->Create();
	document2d = definition->BeginEdit();
	document2d_api7 = kompas->TransferInterface(document2d, kapi::ksAPI7Dual, 0);

	kapi::IViewsAndLayersManagerPtr viewsAndLayersManager(document2d_api7->ViewsAndLayersManager);
	kapi::IViewsPtr views(viewsAndLayersManager->Views);
	view = kapi::IViewPtr(views->ActiveView);
	drawingContainer = kapi::IDrawingContainerPtr(view);
}

void Sketch::endEdit() const {
	definition->EndEdit();
}
