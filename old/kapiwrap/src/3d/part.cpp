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

namespace
{
geom3d::Mesh copyToMesh(ksapi::ITessellationPtr tessellation)
{
    tessellation->RebuildTessellation();

    std::vector<double> points;
    std::vector<int32_t> indexes;
    std::vector<double> normals;
    tessellation->GetFacetPoints(points, indexes, normals);

    geom3d::Mesh mesh;

    mesh.positions.reserve(points.size());
    for (size_t i = 0; i < points.size(); i += 3) {
        mesh.positions.emplace_back(points[i], points[i + 1], points[i + 2]);
    }

    mesh.normals.reserve(normals.size());
    for (size_t i = 0; i < normals.size(); i += 3) {
        mesh.normals.emplace_back(normals[i], normals[i + 1], normals[i + 2]);
    }

    std::copy(indexes.begin(), indexes.end(), std::back_inserter(mesh.indexes));

    return mesh;
}
}

geom3d::Mesh copyToMesh(ksapi::IPartPtr part)
{
    std::vector<ksapi::IFacePtr> faces = getFaces(part);

    geom3d::Mesh mesh;

    for (long iFace = 0; iFace < faces.size(); ++iFace) {
        ksapi::IFacePtr face = faces[iFace];
        ksapi::ITessellationPtr tessellation = face->GetTessellation();

        if (iFace == 0) {
            mesh = copyToMesh(tessellation);
        }
        else {
            geom3d::Mesh faceMesh = copyToMesh(tessellation);
            const geom3d::Mesh::Index pointsCount = static_cast<geom3d::Mesh::Index>(mesh.positions.size());

            mesh.positions.insert(mesh.positions.end(), faceMesh.positions.begin(), faceMesh.positions.end());
            mesh.normals.insert(mesh.normals.end(), faceMesh.normals.begin(), faceMesh.normals.end());

            assert(mesh.positions.size() == mesh.normals.size());
            auto ShiftIndex = std::bind(std::plus(), pointsCount, std::placeholders::_1);
            std::ranges::transform(faceMesh.indexes, faceMesh.indexes.begin(), ShiftIndex);
            mesh.indexes.insert(mesh.indexes.end(), faceMesh.indexes.begin(), faceMesh.indexes.end());
        }
    }

    return mesh;
}

geom3d::Gabarit getGabarit(ksapi::IPartPtr part)
{
    Eigen::Vector3d begin, end;
    part->GetGabarit(false /*full*/, false /*unused*/, begin.x(), begin.y(), begin.z(), end.x(), end.y(), end.z());
    return geom3d::Gabarit(begin, end);
}
