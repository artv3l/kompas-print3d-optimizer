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
	overhangArea,         // Площадь нависающих элементов
	overhangVolume,       // Объем поддерживающих структур
	bottomArea,           // Площадь нижней поверхности
	bottomConvexHullArea, // Площадь выпуклого многоугольника нижней поверхности
	modelHeight,          // Высота модели
	count,                // Кол-во критериев
};

// Составной критерий ориентации детали
enum class OrientationComplexCriteria : uint8_t
{
	overhangArea,          // Площадь нависающих элементов
	overhangAreaAndVolume, // Площадь нависающих элементов и объем поддерживающих структур
	modelHeight,           // Высота модели
	bottomQuality,         // Качество нижней поверхности
	common,                // Общий критерий
	count,                 // Кол-во критериев
};

// Результаты оценки нескольких вариантов ориентации по всем критериям в абсолютных значениях этого критерия
using OrientationsEstimation = std::array<std::vector<double>, enums::toUnderlying(OrientationCriteria::count)>;

/*
  Результаты оценки нескольких вариантов ориентации по всем составным критериям в относительных значениях этого критерия.
  Относительные величины это значения в промежутке [0, 1], где минимальное значение соответствует более лучшей ориентации.
*/
using OrientationsComplexEstimation = std::array<std::vector<double>, enums::toUnderlying(OrientationComplexCriteria::count)>;

// Рассчитать все критерии для нескольких вариантов ориентации
OrientationsEstimation calcOrientationsEstimation(const Mesh& mesh, std::span<const glm::vec3> directions, double overhangThreshold, double offsetThreshold);
// Рассчитать все составные критерии
OrientationsComplexEstimation calcOrientationsComplexEstimation(const OrientationsEstimation& estimation);

struct OrientationStatByMesh final {
	Mesh evalMesh; // Сетка, каждая нормаль которой это оцениваемая ориентация детали
	OrientationsEstimation estimations; // Оценки каждого критерия в абсолютных величинах
	OrientationsComplexEstimation complexEstimations; // Оценки составного каждого критерия в относительных величинах

	std::span<const double> getByCriteria(OrientationCriteria criteria) const;
	// Найти count лучших ориентаций по критерию, возвращает индексы
	std::vector<size_t> findBest(OrientationComplexCriteria criteria, size_t count) const;
};

std::pair<int, int> countPointsOnEachSide(kapi::ksPartPtr part, const PlaneEq& planeEq);
PrintSurface getSelectedPrintSurface(kapi::ksDocument3DPtr document3d);
OrientationStatByMesh calcOrientationStatByMesh(kapi::ksBodyPtr body, double overhangThreshold, double offsetThreshold);
Mesh copyToMesh(kapi::ksTessellationPtr tessellation);
Mesh copyToMesh(kapi::ksBodyPtr body);

// Найти плоскость печати и высоту модели
std::pair<math::Plane, double> calcPrintPlaneAndHeight(const Mesh& mesh, const glm::vec3& direction);


#endif /* PRINT_SURFACE_HPP */
