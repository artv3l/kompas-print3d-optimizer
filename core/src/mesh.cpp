#include "mesh.hpp"

#include <map>

#include <eigen3/Eigen/Dense>

// https://schneide.blog/2016/07/15/generating-an-icosphere-in-c/
namespace
{
using Index = size_t;

struct Triangle
{
	Index vertex[3];
};

using TriangleList = std::vector<Triangle>;
using VertexList = std::vector<Eigen::Vector3d>;

const float X = .525731112119133606f;
const float Z = .850650808352039932f;
const float N = 0.f;

static const VertexList c_vertices =
{
	{-X,N,Z}, {X,N,Z}, {-X,N,-Z}, {X,N,-Z},
	{N,Z,X}, {N,Z,-X}, {N,-Z,X}, {N,-Z,-X},
	{Z,X,N}, {-Z,X, N}, {Z,-X,N}, {-Z,-X, N}
};

static const TriangleList c_triangles =
{
	{0,4,1},{0,9,4},{9,5,4},{4,5,8},{4,8,1},
	{8,10,1},{8,3,10},{5,3,8},{5,2,3},{2,7,3},
	{7,10,3},{7,6,10},{7,11,6},{11,0,6},{0,1,6},
	{6,1,10},{9,0,11},{9,11,2},{9,2,5},{7,2,11}
};

using Lookup = std::map<std::pair<Index, Index>, Index>;

Index vertex_for_edge(Lookup& lookup, VertexList& vertices, Index first, Index second)
{
	Lookup::key_type key(first, second);
	if (key.first > key.second)
		std::swap(key.first, key.second);

	auto inserted = lookup.insert({ key, vertices.size() });
	if (inserted.second)
	{
		auto& edge0 = vertices[first];
		auto& edge1 = vertices[second];
		auto point = (edge0 + edge1).normalized();
		vertices.push_back(point);
	}

	return inserted.first->second;
}

TriangleList subdivide(VertexList& vertices, TriangleList triangles)
{
	Lookup lookup;
	TriangleList result;

	for (auto&& each : triangles)
	{
		std::array<Index, 3> mid;
		for (int edge = 0; edge < 3; ++edge)
		{
			mid[edge] = vertex_for_edge(lookup, vertices,
				each.vertex[edge], each.vertex[(edge + 1) % 3]);
		}

		result.push_back({ each.vertex[0], mid[0], mid[2] });
		result.push_back({ each.vertex[1], mid[1], mid[0] });
		result.push_back({ each.vertex[2], mid[2], mid[1] });
		result.push_back({ mid[0], mid[1], mid[2] });
	}

	return result;
}

using IndexedMesh = std::pair<VertexList, TriangleList>;

IndexedMesh make_icosphere(int subdivisions)
{
	VertexList vertices = c_vertices;
	TriangleList triangles = c_triangles;

	for (int i = 0; i < subdivisions; ++i)
	{
		triangles = subdivide(vertices, triangles);
	}

	return{ vertices, triangles };
}
}

geom3d::Mesh generateIcosphere(uint8_t subdivisionsCount)
{
	auto&& [vertices, triangles] = make_icosphere(subdivisionsCount);

	geom3d::Mesh result;

	result.positions = std::move(vertices);
	vertices = std::vector<Eigen::Vector3d>();

	result.normals.reserve(result.positions.size());
	for (auto&& pos : result.positions)
	{
		result.normals.emplace_back(pos.normalized());
	}

	result.indexes.reserve(triangles.size() * 3);
	for (auto&& triangle : triangles)
	{
		result.indexes.emplace_back(triangle.vertex[0]);
		result.indexes.emplace_back(triangle.vertex[1]);
		result.indexes.emplace_back(triangle.vertex[2]);
	}

	return result;
}
