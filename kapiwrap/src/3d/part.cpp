#include "kapiwrap/3d/part.hpp"

#include <vector>
#include <stdexcept>

#include <KsAPI.h>
#include <ksConstants3D.h>

std::vector<ksapi::IFacePtr> getFaces(ksapi::IPartPtr part)
{
    ksapi::IFeaturePtr feature = part;
    std::vector<ksapi::IFacePtr> faces;
    for (ksapi::IModelObjectPtr obj : feature->GetModelObjects(std::vector<int32_t>{ksObj3dTypeEnum::o3d_face})) {
        faces.emplace_back(obj);
    }
    return faces;
}

std::vector<ksapi::IEdgePtr> getEdges(ksapi::IFacePtr face)
{
    std::vector<ksapi::IEdgePtr> edges;
    for (ksapi::ILoopPtr loop : face->GetLoops()) {
        std::vector<ksapi::IEdgePtr> loopEdges = loop->GetEdges();
        edges.insert(edges.end(), loopEdges.cbegin(), loopEdges.cend());
    }
    return edges;
}
