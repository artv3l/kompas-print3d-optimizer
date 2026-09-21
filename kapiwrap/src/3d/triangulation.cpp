#include "kapiwrap/3d/triangulation.hpp"

#include <microstl.h>

#include "generic/filesystem.hpp"
#include "kapiwrap/3d/part.hpp"
#include "kapiwrap/3d/body.hpp"

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
    for (size_t i = 0; i < points.size(); i += 3)
    {
        mesh.positions.emplace_back(points[i], points[i + 1], points[i + 2]);
    }

    mesh.normals.reserve(normals.size());
    for (size_t i = 0; i < normals.size(); i += 3)
    {
        mesh.normals.emplace_back(normals[i], normals[i + 1], normals[i + 2]);
    }

    std::copy(indexes.begin(), indexes.end(), std::back_inserter(mesh.indexes));

    return mesh;
}
} // namespace

/*
    API не позволяет напрямую получить информацию о сетке полигонального объекта.
    Поэтому, для получения сеток ПгО, используем костыль:
    - Конвертируем все ПгО файла в stl в временную папку
    - Читаем stl файл и загружаем сетку оттуда
*/
geom3d::Mesh getMeshObjectsTriangulation(ksapi::IKompasDocument3DPtr document3d)
{
    ksapi::IPartPtr part = document3d->GetTopPart();
    ksapi::IFeaturePtr feature = part;
    auto meshObjects = feature->GetModelObjects(std::vector<int32_t>{ksObj3dTypeEnum::o3d_MeshObject3D});
    if (meshObjects.empty())
        return geom3d::Mesh();

    const auto filepath = filesystem::getTempFile().replace_extension("stl");

    ksapi::IAdditionConvertParametersPtr params =
        document3d->GetInterface(KompasAPIObjectTypeEnum::ksObjectAdditionConvertParameters);
    params->SetFormat(ksKOMPASConverterEnum::ksConverterToSTL);
    params->SetExportObjects(meshObjects);
    document3d->ConvertToAdditionFormat(filepath, params);

    microstl::MeshReaderHandler meshHandler;
    if (microstl::Result result = microstl::Reader::readStlFile(filepath, meshHandler);
        result != microstl::Result::Success)
    {
        assert(false);
        return geom3d::Mesh();
    }
    const microstl::Mesh & mesh = meshHandler.mesh;

    geom3d::Mesh result;
    result.positions.reserve(mesh.facets.size() * 3);
    result.normals.reserve(mesh.facets.size() * 3);
    result.indexes.reserve(mesh.facets.size() * 3);
    size_t index = 0;
    for (const microstl::Facet & facet : mesh.facets)
    {
        result.positions.emplace_back(facet.v1.x, facet.v1.y, facet.v1.z);
        result.normals.emplace_back(facet.n.x, facet.n.y, facet.n.z);
        result.indexes.emplace_back(index++);

        result.positions.emplace_back(facet.v2.x, facet.v2.y, facet.v2.z);
        result.normals.emplace_back(facet.n.x, facet.n.y, facet.n.z);
        result.indexes.emplace_back(index++);

        result.positions.emplace_back(facet.v3.x, facet.v3.y, facet.v3.z);
        result.normals.emplace_back(facet.n.x, facet.n.y, facet.n.z);
        result.indexes.emplace_back(index++);
    }

    const bool isRemoved = std::filesystem::remove(filepath);
    assert(isRemoved);

    return result;
}

geom3d::Mesh getBodyTriangulation(ksapi::IBodyPtr body)
{
    geom3d::Mesh result;

    auto faces = getBodyFaces(body);
    for (auto && face : faces)
    {
        ksapi::ITessellationPtr tessellation = face->GetTessellation();
        geom3d::Mesh tessMesh = copyToMesh(tessellation);
        geom3d::mergeMeshes(result, tessMesh);
    }
    return result;
}
