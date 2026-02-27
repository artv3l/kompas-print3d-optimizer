#ifndef PRINT_SURFACE_HPP
#define PRINT_SURFACE_HPP

#include <utility>

#include <glm/vec3.hpp>

#include "oglwrap/Mesh.hpp"
#include "generic/math.hpp"


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

struct OrientationStatByMesh final {
	kapi::ksBodyPtr body; // Деталь, для которой проводится анализ
	double bodyArea; // Площадь детали

	Mesh evalMesh; // Сетка, каждая нормаль которой это оцениваемая ориентация детали
	std::vector<double> overhangsArea; // Площади нависаний для каждого вектора из m_evalMesh

	std::vector<double> printSurfacesArea; // Площади поверхностей печати для каждого вектора из m_evalMesh

	// ? Максимальный угол нависаний
};

std::pair<int, int> countPointsOnEachSide(kapi::ksPartPtr part, const PlaneEq& planeEq);
PrintSurface getSelectedPrintSurface(kapi::ksDocument3DPtr document3d);
OrientationStatByMesh calcOrientationStatByMesh(kapi::ksBodyPtr body, double overhangThreshold);
Mesh copyToMesh(kapi::ksTessellationPtr tessellation);
Mesh copyToMesh(kapi::ksBodyPtr body);
math::Plane calcPrintPlane(const Mesh& mesh, const glm::vec3& direction);

#endif /* PRINT_SURFACE_HPP */
