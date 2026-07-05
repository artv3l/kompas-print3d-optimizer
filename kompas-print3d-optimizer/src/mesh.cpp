#include "mesh.hpp"

#include <cassert>
#include <algorithm>
#include <vector>
#include <iterator>

#include "windows.hpp"

geom3d::Mesh copyToMesh(kapi::ksTessellationPtr tessellation)
{
    tessellation->refresh();

    _variant_t pointsVariant, indexesVariant, normalsVariant;
    tessellation->GetFacetPoints(&pointsVariant, &indexesVariant);
    tessellation->GetFacetNormals(&normalsVariant);
    auto&& [points, pointsLock] = getSafeArrayData<geom3d::Vec3>(pointsVariant);
    auto&& [normals, normalsLock] = getSafeArrayData<geom3d::Vec3>(normalsVariant);
    auto&& [indexes, indexesLock] = getSafeArrayData<int>(indexesVariant);

    geom3d::Mesh mesh;

    mesh.positions.reserve(points.size());
    std::copy(points.begin(), points.end(), std::back_inserter(mesh.positions));

    mesh.normals.reserve(normals.size());
    std::copy(normals.begin(), normals.end(), std::back_inserter(mesh.normals));

    std::copy(indexes.begin(), indexes.end(), std::back_inserter(mesh.indexes));

    return mesh;
}

geom3d::Mesh copyToMesh(kapi::ksBodyPtr body)
{
    kapi::ksFaceCollectionPtr faces = body->FaceCollection();

    geom3d::Mesh mesh;

    for (long iFace = 0, nFaces = faces->GetCount(); iFace < nFaces; ++iFace) {
        kapi::ksFaceDefinitionPtr face = faces->GetByIndex(iFace);
        kapi::ksTessellationPtr tessellation = face->GetTessellation();

        if (iFace == 0) {
            mesh = copyToMesh(tessellation);
        } else {
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
