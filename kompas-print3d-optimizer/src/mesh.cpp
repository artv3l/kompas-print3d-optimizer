#include "mesh.hpp"

#include <cassert>
#include <algorithm>
#include <vector>
#include <iterator>

#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector3.h>

#include "windows.hpp"

Mesh generateIcosphere(uint8_t subdivisionsCount)
{
    auto icosphere = Magnum::Primitives::icosphereSolid(subdivisionsCount);
    
    auto positions = icosphere.positions3DAsArray();
    auto normals = icosphere.normalsAsArray();
    if (positions.size() != normals.size())
    {
        assert(false);
        return {};
    }

    auto toGlmVec = [](const Magnum::Vector3 & vec3) { return glm::vec3(vec3.x(), vec3.y(), vec3.z()); };

    Mesh result;

    result.positions.reserve(positions.size());
    std::transform(positions.begin(), positions.end(), std::back_inserter(result.positions), toGlmVec);

    result.normals.reserve(normals.size());
    std::transform(normals.begin(), normals.end(), std::back_inserter(result.normals), toGlmVec);

    auto indexes = icosphere.indicesAsArray();
    result.indexes.reserve(indexes.size());
    std::copy(indexes.begin(), indexes.end(), std::back_inserter(result.indexes));

    return result;
}

namespace // https://www.geeksforgeeks.org/cpp/convex-hull-algorithm-in-cpp
{
// Function to calculate the cross product of vectors OA and OB
// Returns positive for a counterclockwise turn and negative for a clockwise turn
double cross_product(Eigen::Vector2d O, Eigen::Vector2d A, Eigen::Vector2d B)
{
    return (A.x() - O.x()) * (B.y() - O.y()) - (A.y() - O.y()) * (B.x() - O.x());
}

// Function to return a list of points on the convex hull in counterclockwise order
std::vector<Eigen::Vector2d> convex_hull(std::vector<Eigen::Vector2d> A)
{
    int n = static_cast<int>(A.size()), k = 0;

    // If there are 3 or fewer points, return them as the convex hull
    if (n <= 3)
        return A;

    // Initialize a vector to store the convex hull points
    std::vector<Eigen::Vector2d> ans(2 * n);

    // Sort the points lexicographically
    std::sort(A.begin(), A.end(), [](const Eigen::Vector2d & a, const Eigen::Vector2d & b)
        {
            return a.x() < b.x() || (a.x() == b.x() && a.y() < b.y());
        }
    );

    // Build the lower hull
    for (int i = 0; i < n; ++i)
    {
        // Remove the last point if it is not part of the convex hull
        // Check if the cross product indicates a clockwise turn
        while (k >= 2 && cross_product(ans[k - 2], ans[k - 1], A[i]) <= 0)
            k--;
        ans[k++] = A[i];
    }

    // Build the upper hull
    for (size_t i = n - 1, t = k + 1; i > 0; --i)
    {
        // Remove the last point if it is not part of the convex hull
        // Check if the cross product indicates a clockwise turn
        while (k >= t && cross_product(ans[k - 2], ans[k - 1], A[i - 1]) <= 0)
            k--;
        ans[k++] = A[i - 1];
    }

    // Resize the vector to remove any extra elements
    ans.resize(k - 1);

    return ans;
}
}

std::vector<Eigen::Vector2d> convexHull(std::span<Eigen::Vector2d> points)
{
    return convex_hull(std::vector<Eigen::Vector2d>(points.begin(), points.end()));
}

Mesh copyToMesh(kapi::ksTessellationPtr tessellation)
{
    tessellation->refresh();

    _variant_t pointsVariant, indexesVariant, normalsVariant;
    tessellation->GetFacetPoints(&pointsVariant, &indexesVariant);
    tessellation->GetFacetNormals(&normalsVariant);
    auto&& [points, pointsLock] = getSafeArrayData<glm::dvec3>(pointsVariant);
    auto&& [normals, normalsLock] = getSafeArrayData<glm::dvec3>(normalsVariant);
    auto&& [indexes, indexesLock] = getSafeArrayData<int>(indexesVariant);

    auto toFloatVec = [](const glm::dvec3& dvec3) { return glm::vec3(dvec3); };

    Mesh mesh;

    mesh.positions.reserve(points.size());
    std::transform(points.begin(), points.end(), std::back_inserter(mesh.positions), toFloatVec);

    mesh.normals.reserve(normals.size());
    std::transform(normals.begin(), normals.end(), std::back_inserter(mesh.normals), toFloatVec);

    std::copy(indexes.begin(), indexes.end(), std::back_inserter(mesh.indexes));

    return mesh;
}

Mesh copyToMesh(kapi::ksBodyPtr body)
{
    kapi::ksFaceCollectionPtr faces = body->FaceCollection();

    Mesh mesh;

    for (long iFace = 0, nFaces = faces->GetCount(); iFace < nFaces; ++iFace) {
        kapi::ksFaceDefinitionPtr face = faces->GetByIndex(iFace);
        kapi::ksTessellationPtr tessellation = face->GetTessellation();

        if (iFace == 0) {
            mesh = copyToMesh(tessellation);
        } else {
            Mesh faceMesh = copyToMesh(tessellation);
            const Index pointsCount = static_cast<Index>(mesh.positions.size());

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
