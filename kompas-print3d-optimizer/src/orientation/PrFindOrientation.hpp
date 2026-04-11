#pragma once

#include <optional>

#include "kapiwrap/PropertyManagerObject.hpp"
#include "settings/DocumentData.hpp"

enum class Accuracy : uint8_t
{
	low,
	medium,
	high,
	count
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

class PrFindOrientation : public PropertyManagerObject
{
public:
	PrFindOrientation(kapi::KompasObjectPtr kompas, DocumentData& documentData);

private:
	virtual bool buttonClick(long buttonId) override;
	virtual bool changeControlValue(IDispatch* control);
	virtual bool controlCommand(IDispatch* control, long buttonId);
	virtual bool selectItem(IDispatch* control, long index, bool select);

private:
	void initControls();
	void updateControls();
	void refillGrid(std::span<const size_t> indexes);
	void updateHeatmap();
	void updateScene();

	DocumentData& m_documentData;

	kapi::IPropertyTabPtr m_mainTab;
	kapi::IPropertyControlsPtr m_controls;

	struct
	{
		kapi::IPropertyEditPtr overhangThreshold;
		kapi::IPropertyEditPtr bottomThreshold;
		kapi::IPropertyEditPtr resultCount;
		kapi::IPropertyListPtr accuracy;
	} m_ctrls;

	kapi::IPropertyListPtr m_metricsList;
	kapi::IPropertyCheckBoxPtr m_visualizeCheckBox;
	kapi::IPropertyTextButtonPtr m_recalcButton;
	kapi::IPropertyGridPtr m_resultGrid;

	OrientationComplexCriteria m_criteria = OrientationComplexCriteria::common;
	bool m_isShowHeatmap = false;
	double m_overhangThreshold = 0.0;
	double m_bottomThreshold = 0.0;
	size_t m_currentGridRow = 0; // 0 - строка не выбрана
	std::vector<size_t> m_orientationsInGrid;
	size_t m_resultCount = 0; // Кол-во вариантов ориентаций для вывода в таблицу
	Accuracy m_accuracy;

	std::unique_ptr<OrientationStatByMesh> m_stat;
};
