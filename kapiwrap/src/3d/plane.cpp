#include "kapiwrap/3d/plane.hpp"

ksapi::IModelObjectPtr createPlanePerpendicular(ksapi::IPartPtr part, ksapi::IEdgePtr edge, ksapi::IVertexPtr vertex, bool isHidden)
{
	ksapi::IAuxiliaryGeomContainerPtr auxGeomCont = part;
	ksapi::IPlanes3DPtr planes = auxGeomCont->GetPlanes3D();
	
	ksapi::IPlane3DPerpendicularByEdgePtr plane = planes->Add(ksObj3dTypeEnum::o3d_planePerpendicular);
	plane->SetEdge(edge);
	plane->SetPoint(vertex);
	plane->SetHidden(isHidden);
	plane->Update();

	return plane;
}
