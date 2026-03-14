#ifndef PRINT_SURFACE_HPP
#define PRINT_SURFACE_HPP

#include <utility>
#include <span>

#include <glm/vec3.hpp>

#include "oglwrap/Mesh.hpp"
#include "generic/math.hpp"
#include "generic/enums.hpp"


struct PlaneEq {
	double a, b, c, d;

	PlaneEq(kapi::ksFaceDefinitionPtr face);

	bool operator==(const PlaneEq& other) const;
	bool operator!=(const PlaneEq& other) const;

	void invert();
};

struct PrintSurface {
	kapi::ksFaceDefinitionPtr face;
	PlaneEq eq;
};

// Критерий оценки ориентации детали
enum class OrientationCriteria : uint8_t
{
	overhangArea,   // Площадь нависаний
	overhangVolume, // Объем под нависаниями
	bottomArea,     // Площадь нижней грани
	count,          // Кол-во критериев
};

// Результаты оценки нескольких вариантов ориентации по всем критериям
using OrientationsEstimation = std::array<std::vector<double>, enums::toUnderlying(OrientationCriteria::count)>;

// Рассчитать все критерии для нескольких вариантов ориентации
OrientationsEstimation calcOrientationsEstimation(const Mesh& mesh, std::span<const glm::vec3> directions, double overhangThreshold, double offsetThreshold);

struct OrientationStatByMesh final {
	Mesh evalMesh; // Сетка, каждая нормаль которой это оцениваемая ориентация детали
	OrientationsEstimation estimations;

	std::span<const double> getByCriteria(OrientationCriteria criteria) const;
};

std::pair<int, int> countPointsOnEachSide(kapi::ksPartPtr part, const PlaneEq& planeEq);
PrintSurface getSelectedPrintSurface(kapi::ksDocument3DPtr document3d);
OrientationStatByMesh calcOrientationStatByMesh(kapi::ksBodyPtr body, double overhangThreshold);
Mesh copyToMesh(kapi::ksTessellationPtr tessellation);
Mesh copyToMesh(kapi::ksBodyPtr body);
math::Plane calcPrintPlane(const Mesh& mesh, const glm::vec3& direction);


#endif /* PRINT_SURFACE_HPP */
