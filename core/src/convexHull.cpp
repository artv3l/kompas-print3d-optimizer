#include "convexHull.hpp"

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

        // Initialize a vector to store the convex hull points
        std::vector<Eigen::Vector2d> ans(2 * n);

        // Sort the points lexicographically
        std::sort(A.begin(), A.end(), [](const Eigen::Vector2d& a, const Eigen::Vector2d& b)
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

        return (ans.size() >= 3) ? ans : std::vector<Eigen::Vector2d>();
    }
}

std::vector<Eigen::Vector2d> convexHull(std::span<const Eigen::Vector2d> points)
{
    return convex_hull(std::vector<Eigen::Vector2d>(points.begin(), points.end()));
}
