#include "Mesh.hpp"

namespace
{
template <typename T1, typename T2>
std::vector<T1> staticCastEach(std::span<const T2> span)
{
	std::vector<T1> result;
	result.reserve(span.size());
	for (auto&& item : span) {
		result.emplace_back(static_cast<T1>(item));
	}
	return result;
}

std::vector<glm::vec3> toGlm(std::span<const geom3d::Vec3> points)
{
	std::vector<glm::vec3> result;
	result.reserve(points.size());
	for (auto&& point : points) {
		result.emplace_back(
			static_cast<float>(point.x()),
			static_cast<float>(point.y()),
			static_cast<float>(point.z())
		);
	}
	return result;
}
}

Mesh::Mesh(const geom3d::Mesh& mesh):
	positions(toGlm(mesh.positions)),
	normals(toGlm(mesh.normals)),
	indexes(staticCastEach<Index, geom3d::Mesh::Index>(mesh.indexes))
{
}

ColoredMesh::ColoredMesh(const geom3d::Mesh& mesh, const color::RGB& color) :
	Mesh(mesh),
	colors(mesh.positions.size(), glm::vec3(color.red, color.green, color.blue))
{
}

Polyline3D::Polyline3D(std::span<const geom3d::Vec3> points, const color::RGB& color):
	m_points(toGlm(points)),
	m_color(color.red, color.green, color.blue)
{
}
