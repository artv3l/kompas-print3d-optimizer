#include "mesh.hpp"

#include <cassert>
#include <algorithm>
#include <vector>
#include <iterator>

#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector3.h>

Mesh generateIcosphere()
{
    auto icosphere = Magnum::Primitives::icosphereSolid(2);
    
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
// Structure to represent a point in 2D space
struct Point
{
    double x, y;

    // Operator overloading to compare two points lexicographically
    bool operator<(Point p)
    {
        return x < p.x || (x == p.x && y < p.y);
    }
};

// Function to calculate the cross product of vectors OA and OB
// Returns positive for a counterclockwise turn and negative for a clockwise turn
double cross_product(Point O, Point A, Point B)
{
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

// Function to return a list of points on the convex hull in counterclockwise order
std::vector<Point> convex_hull(std::vector<Point> A)
{
    int n = A.size(), k = 0;

    // If there are 3 or fewer points, return them as the convex hull
    if (n <= 3)
        return A;

    // Initialize a vector to store the convex hull points
    std::vector<Point> ans(2 * n);

    // Sort the points lexicographically
    sort(A.begin(), A.end());

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

geometry::Polygon convexHull(std::span<glm::vec2> points)
{
    std::vector<Point> pts(points.size(), Point(0, 0));
    for (size_t i = 0; i < points.size(); ++i) {
        pts[i] = Point(points[i].x, points[i].y);
    }

    std::vector<Point> result = convex_hull(pts);
    geometry::Polygon polygon(result.size(), geometry::Vector2D(0, 0));
    for (size_t i = 0; i < result.size(); ++i) {
        polygon.setPoint(i, geometry::Vector2D(result[i].x, result[i].y));
    }
    return polygon;
}
