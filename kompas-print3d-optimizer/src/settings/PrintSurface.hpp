#ifndef PRINT_SURFACE_HPP
#define PRINT_SURFACE_HPP

#include <utility>
#include <span>
#include <memory>

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

enum class OrientationComplexCriteria : uint8_t
{
	overhangs,     // Количество поддержек
	bottomQuality, // Качество нижней поверхности
	common,        // Общий критерий
	count,         // Кол-во критериев
};

// Контур нижней поверхности: точка, отрезок или convex_hull
using BottomContour = std::vector<glm::vec3>;

enum class TriangleProperties : uint8_t
{
	none,     // Обычный треугольник
	overhang, // Нависающий
	bottom,   // Принадлежит нижней повехрности
};

// Измерения для одной ориентации
struct OrientationInfo final
{
	double overhangArea = 0.0;         // Площадь нависающих элементов
	double overhangVolume = 0.0;       // Объем поддерживающих структур
	double bottomArea = 0.0;           // Площадь нижней поверхности
	double bottomConvexHullArea = 0.0; // Площадь выпуклого многоугольника нижней поверхности
	double modelHeight = 0.0;          // Высота модели
	BottomContour bottomContour;       // Контур нижней поверхности (convex hull)
	std::vector<TriangleProperties> triangleProperties; // Свойства всех треугольников модели
};

/*
  Результаты оценки нескольких вариантов ориентации по всем составным критериям в относительных значениях этого критерия.
  Относительные величины это значения в промежутке [0, 1], где минимальное значение соответствует более лучшей ориентации.
  Относительные критерии существуют только в контексте сравнения нескольких вариантов ориенатции, поэтому тут массивы.
*/
using OrientationComplexInfos = std::array<std::vector<double>, enums::toUnderlying(OrientationComplexCriteria::count)>;

struct OrientationStatByMesh final {
	std::shared_ptr<ColoredMesh> model; // Оцениваемая модель
	Mesh evalMesh; // Сетка, каждая нормаль которой это оцениваемая ориентация детали
	std::vector<OrientationInfo> infos; // Измерения для всех ориентаций
	OrientationComplexInfos complexInfos;

	// Найти count лучших ориентаций по критерию, возвращает индексы
	std::vector<size_t> findBest(OrientationComplexCriteria criteria, size_t count) const;
	// Обновить закраску модели по индексу ориентации
	void updateMeshColors(size_t index);
};

std::pair<int, int> countPointsOnEachSide(kapi::ksPartPtr part, const PlaneEq& planeEq);
PrintSurface getSelectedPrintSurface(kapi::ksDocument3DPtr document3d);
OrientationStatByMesh calcOrientationStatByMesh(kapi::ksBodyPtr body, double overhangThreshold, double offsetThreshold, size_t subdivisionsCount);
Mesh copyToMesh(kapi::ksTessellationPtr tessellation);
Mesh copyToMesh(kapi::ksBodyPtr body);

// Найти плоскость печати и высоту модели
std::pair<math::Plane, double> calcPrintPlaneAndHeight(const Mesh& mesh, const glm::vec3& direction);


#endif /* PRINT_SURFACE_HPP */
