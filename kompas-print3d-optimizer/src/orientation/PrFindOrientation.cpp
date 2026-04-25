#include "PrFindOrientation.hpp"

#include <algorithm>
#include <format>
#include <ranges>
#include <numeric>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "kapiwrap/3d/body.hpp"
#include "settings/SettingInitializer.hpp"
#include "generic/color.hpp"
#include "utils.hpp"
#include "settings/SettingsManager.hpp"
#include "global.hpp"
#include "windows.hpp"
#include "generic/perfomance.hpp"

namespace
{
const std::unordered_map<Accuracy, std::wstring_view> c_accuracyNames = {
	{Accuracy::low, L"Низкая"},
	{Accuracy::medium, L"Средняя"},
	{Accuracy::high, L"Высокая"}
};

uint8_t getSubdivisionsCount(Accuracy accuracy)
{
	return enums::toUnderlying(accuracy) + 1;
}

const std::unordered_map<OrientationComplexCriteria, std::wstring_view> c_metricNames = {
	{OrientationComplexCriteria::overhangs, L"Количество поддержек"},
	{OrientationComplexCriteria::bottomQuality, L"Нижняя поверхность"},
	{OrientationComplexCriteria::common, L"Общий"}
};

void createLocalCS(kapi::ksPartPtr part, const math::Plane & plane)
{
	kapi::IPart7Ptr part7 = global::kompas->TransferInterface(part, kapi::ksAPITypeEnum::ksAPI7Dual, 0);
	kapi::IAuxiliaryGeomContainerPtr auxGeomCont(part7);
	kapi::ILocalCoordinateSystemsPtr localCSs(auxGeomCont->LocalCoordinateSystems);

	const math::Placement printPlanePlacement = math::Placement::createByAxisZ(
		math::project(glm::vec3(0, 0, 0), plane), plane.getNormal());
	const glm::mat4 matrix = printPlanePlacement.matrixToWorld();

	std::vector<double> matrixVector;
	for (uint8_t row = 0; row < 4; ++row) {
		for (uint8_t col = 0; col < 4; ++col) {
			matrixVector.push_back(static_cast<double>(matrix[row][col]));
		}
	}

	kapi::ILocalCoordinateSystemPtr localCS(localCSs->Add());
	localCS->InitByMatrix3D(toVariant(matrixVector));
	localCS->Update();
}

// Найти плоскость печати и высоту модели
std::pair<math::Plane, double> calcPrintPlaneAndHeight(const Mesh& mesh, const glm::vec3& direction)
{
	auto toShift = [normDir = glm::normalize(direction)](const glm::vec3& vec)
		{
			return -glm::dot(vec, normDir);
		};

	const auto [min, max] = std::ranges::minmax_element(mesh.positions, {}, toShift);
	if (min == mesh.positions.end() || max == mesh.positions.end())
		throw std::logic_error(""); // TODO

	const math::Plane printPlane(direction, *min);
	return std::make_pair(printPlane, math::distance(*max, printPlane));
}

OrientationInfo calcOrientationInfo(const Mesh& mesh, const glm::vec3& direction, double overhangThreshold, double offsetThreshold)
{
	OrientationInfo info;

	const double overhangThresholdRad = degreeToRadian(overhangThreshold);
	const auto [printPlane, height] = calcPrintPlaneAndHeight(mesh, direction);

	const math::Placement printPlanePlacement = math::Placement::createByAxisZ(
		math::project(glm::vec3(0, 0, 0), printPlane), printPlane.getNormal()
	);
	const glm::mat4 toWorld = printPlanePlacement.matrixToWorld();
	const glm::mat4 toPrintPlanePlacement = math::worldToLocal(printPlanePlacement);

	std::vector<glm::vec2> convexHullPoints;
	convexHullPoints.reserve(mesh.positions.size());

	info.triangleProperties.resize(mesh.indexes.size() / 3);

	for (int iIndex = 0; (iIndex + 2) < mesh.indexes.size(); iIndex += 3) {
		const size_t i1 = mesh.indexes[iIndex];
		const size_t i2 = mesh.indexes[iIndex + 1];
		const size_t i3 = mesh.indexes[iIndex + 2];
		const math::Triangle triangle(mesh.positions[i1], mesh.positions[i2], mesh.positions[i3]);

		// Для всех трех точек будут одинаковые нормали, поэтому берем любую (первую)
		const glm::vec3 normal = mesh.normals[i1];
		const double angleRad = calcAngleBetween(normal, printPlane.getNormal());
		const double triangleArea = triangle.area();

		if (isOnPrintPlane(triangle, printPlane, offsetThreshold)) {
			info.bottomArea += triangleArea;
			info.triangleProperties[iIndex / 3] = TriangleProperties::bottom;
		}
		else if (angleRad < overhangThresholdRad) {
			// Площадь под мостами тоже считаем за площадь нависаний
			info.overhangArea += triangleArea;
			info.overhangVolume += volumeUnderOverhang(printPlane, triangle);
			info.triangleProperties[iIndex / 3] = TriangleProperties::overhang;
		}

		for (auto&& pnt : triangle.points) {
			const glm::vec4 pntLocal = toPrintPlanePlacement * glm::vec4(pnt, 1.0f);
			if (std::abs(pntLocal.z) < offsetThreshold) {
				convexHullPoints.emplace_back(pntLocal.x, pntLocal.y);
			}
		}
	}

	info.modelHeight = height;

	if (convexHullPoints.size() >= 3) {
		geometry::Polygon hullPolygon = convexHull(convexHullPoints);
		info.bottomConvexHullArea = hullPolygon.area();

		for (auto&& pnt : hullPolygon.m_points) {
			info.bottomContour.push_back(toWorld * glm::vec4(pnt.x, pnt.y, 0.0f, 1.0f));
		}
	}
	else {
		for (auto&& pnt : convexHullPoints) {
			info.bottomContour.push_back(toWorld * glm::vec4(pnt.x, pnt.y, 0.0f, 1.0f));
		}
	}

	return info;
}

// Рассчитать все критерии для нескольких вариантов ориентации
std::vector<OrientationInfo> calcOrientationsEstimation(const Mesh& mesh, std::span<const glm::vec3> directions, double overhangThreshold, double offsetThreshold)
{
	assert(mesh.indexes.size() % 3 == 0);

	auto perfLock = perfomance::measureTime([](std::chrono::nanoseconds time) {
		std::cout << "Simple criteria calc: "
			      << std::chrono::duration_cast<std::chrono::milliseconds>(time)
			      << "\n";
	});

	std::vector<OrientationInfo> result;
	result.resize(directions.size());
	for (size_t i = 0; i < directions.size(); ++i) {
		result[i] = calcOrientationInfo(mesh, directions[i], overhangThreshold, offsetThreshold);
	}
	return result;
}

// Преобразовать абсолюные значения в относительные [0, 1]
template <std::ranges::range R>
std::vector<double> toRelative(R absoluteValues)
{
	std::vector<double> relativeValues(absoluteValues.size(), 0.0);

	const auto [min, max] = std::ranges::minmax_element(absoluteValues);
	if (min == absoluteValues.end() || max == absoluteValues.end())
		throw std::logic_error(""); // TODO

	auto convert = std::bind(math::convertRanges, std::placeholders::_1, *min, *max, 0.0, 1.0);
	std::ranges::transform(absoluteValues, relativeValues.begin(), convert);

	return relativeValues;
}

// Рассчитать все составные критерии
OrientationComplexInfos calcOrientationsComplexEstimation(std::span<OrientationInfo> infos)
{
	auto perfLock = perfomance::measureTime([](std::chrono::nanoseconds time) {
		std::cout << "Composite criteria calc: "
			      << std::chrono::duration_cast<std::chrono::milliseconds>(time)
			      << "\n";
	});

	size_t size = infos.size();

	OrientationComplexInfos result;
	std::ranges::for_each(result, [size](auto& vector) { vector.resize(size); });

	auto& overhangs = result[enums::toUnderlying(OrientationComplexCriteria::overhangs)];
	{
		auto relOverhangAreas = toRelative(infos | std::views::transform(&OrientationInfo::overhangArea));
		auto relOverhangVolumes = toRelative(infos | std::views::transform(&OrientationInfo::overhangVolume));
		for (size_t i = 0; i < size; ++i) {
			overhangs[i] = (0.6 * relOverhangVolumes[i]) + (0.4 * relOverhangAreas[i]);
		}
	}

	auto& bottomQuality = result[enums::toUnderlying(OrientationComplexCriteria::bottomQuality)];
	{
		auto relBottomAreas = toRelative(infos | std::views::transform(&OrientationInfo::bottomArea));
		auto relBottomConvexHullAreas = toRelative(infos | std::views::transform(&OrientationInfo::bottomConvexHullArea));
		for (size_t i = 0; i < size; ++i) {
			bottomQuality[i] = (0.6 * (1.0 - relBottomAreas[i])) + (0.4 * (1.0 - relBottomConvexHullAreas[i]));
		}
	}

	{
		auto relModelHeight = toRelative(infos | std::views::transform(&OrientationInfo::modelHeight));
		auto& common = result[enums::toUnderlying(OrientationComplexCriteria::common)];
		for (size_t i = 0; i < size; ++i) {
			common[i] = (0.4 * overhangs[i]) + (0.4 * bottomQuality[i]) + (0.2 * relModelHeight[i]);
		}
	}

	return result;
}

OrientationStatByMesh calcOrientationStatByMesh(kapi::ksBodyPtr body, double overhangThreshold, double offsetThreshold, uint8_t subdivisionsCount)
{
	Mesh mesh = copyToMesh(body);

	OrientationStatByMesh result;

	result.model = std::make_shared<ColoredMesh>();
	result.model->positions = std::move(mesh.positions);
	result.model->normals = std::move(mesh.normals);
	result.model->indexes = std::move(mesh.indexes);
	result.model->colors = std::vector<glm::vec3>(result.model->positions.size(), glm::vec3(0.8f, 0.8f, 0.8f));

	result.evalMesh = generateIcosphere(subdivisionsCount);
	result.infos = calcOrientationsEstimation(*result.model, result.evalMesh.normals, overhangThreshold, offsetThreshold);
	result.complexInfos = calcOrientationsComplexEstimation(result.infos);
	return result;
}
}

std::vector<size_t> OrientationStatByMesh::findBest(OrientationComplexCriteria criteria, size_t count) const
{
	const auto& complexEstimation = complexInfos[enums::toUnderlying(criteria)];
	std::vector<size_t> indexes(complexEstimation.size());
	std::iota(indexes.begin(), indexes.end(), 0);

	auto indexToElem = [&complexEstimation, criteria](size_t index)
		{
			return complexEstimation[index];
		};
	std::ranges::partial_sort(indexes, indexes.begin() + count, {}, indexToElem);

	return std::vector<size_t>(indexes.begin(), indexes.begin() + count);
}

void OrientationStatByMesh::updateMeshColors(size_t index)
{
	const auto& props = infos[index].triangleProperties;
	assert(props.size() == (model->indexes.size() / 3));

	for (size_t i = 0; i < props.size(); ++i) {
		const size_t i1 = model->indexes[i * 3];
		const size_t i2 = model->indexes[i * 3 + 1];
		const size_t i3 = model->indexes[i * 3 + 2];

		glm::vec3 color(0.8f, 0.8f, 0.8f);
		if (props[i] == TriangleProperties::overhang) {
			color = glm::vec3(1.0f, 0.0f, 0.0f);
		}
		else if (props[i] == TriangleProperties::bottom) {
			color = glm::vec3(0.0f, 0.0f, 1.0f);
		}

		model->colors[i1] = color;
		model->colors[i2] = color;
		model->colors[i3] = color;
	}
}

PrFindOrientation::PrFindOrientation(kapi::KompasObjectPtr kompas, DocumentData& documentData):
	PropertyManagerObject(kompas),
	m_documentData(documentData),
	m_mainTab(m_propertyManager->PropertyTabs->Add("MainTab")),
	m_controls(m_mainTab->PropertyControls)
{
	m_propertyManager->Layout = kapi::PropertyManagerLayout::pmAlignRight;
	m_propertyManager->SpecToolbar = kapi::SpecPropertyToolBarEnum::pnEnterEscHelp;
	m_propertyManager->Caption = L"Поиск плоскости печати";

	m_overhangThreshold = m_documentData.getSettings()->getDoubleSetting(si::overhangThreshold.name)->getValue();
	m_bottomThreshold = 0.2;
	m_resultCount = 5;
	m_accuracy = Accuracy::medium;

	initControls();
	updateControls();
}

bool PrFindOrientation::buttonClick(long buttonId)
{
	auto hm = m_documentData.getHighlightingManager();
	hm->cleanObjects();

	switch (buttonId) {
	case kapi::SpecPropertyButtonEnum::pbEnter:
	{
		// TODO Синхронизация максимального угла нависаний с переменными

		// Создание ЛСК плоскости печати
		const glm::vec3 normal = m_stat->evalMesh.normals[m_orientationsInGrid[m_currentGridRow - 1]];
		const glm::vec3 point = m_stat->infos[m_orientationsInGrid[m_currentGridRow - 1]].bottomContour[0];
		const math::Plane plane(normal, point);
		kapi::ksPartPtr part = m_documentData.getDocument()->GetPart(kapi::Part_Type::pTop_Part);
		createLocalCS(part, plane);

		hide();
		break;
	}
	case kapi::SpecPropertyButtonEnum::pbHelp:
		break;
	case kapi::SpecPropertyButtonEnum::pbEsc:
		hide();
		break;
	}
	return true;
}

bool PrFindOrientation::changeControlValue(IDispatch* control)
{
	for (size_t i = 0; i < enums::toUnderlying(OrientationComplexCriteria::count); ++i) {
		if (static_cast<_bstr_t>(m_metricsList->Value) == _bstr_t(c_metricNames.at(static_cast<OrientationComplexCriteria>(i)).data())) {
			m_criteria = static_cast<OrientationComplexCriteria>(i);
			break;
		}
	}
	for (size_t i = 0; i < enums::toUnderlying(Accuracy::count); ++i) {
		if (static_cast<_bstr_t>(m_ctrls.accuracy->Value) == _bstr_t(c_accuracyNames.at(static_cast<Accuracy>(i)).data())) {
			m_accuracy = static_cast<Accuracy>(i);
			break;
		}
	}

	m_isShowHeatmap = static_cast<bool>(m_visualizeCheckBox->Value);

	m_overhangThreshold = m_ctrls.overhangThreshold->Value;
	m_bottomThreshold = m_ctrls.bottomThreshold->Value;
	m_resultCount = m_ctrls.resultCount->Value;

	updateControls();

	return false; /* unused */
}

bool PrFindOrientation::controlCommand(IDispatch* control, long buttonId)
{
	if (buttonId == m_recalcButton->Id) {
		kapi::ksPartPtr part = m_documentData.getDocument()->GetPart(kapi::Part_Type::pTop_Part);
		m_stat = std::make_unique<OrientationStatByMesh>(calcOrientationStatByMesh(part->GetMainBody(),
			m_overhangThreshold, m_bottomThreshold, getSubdivisionsCount(m_accuracy)
		));

		updateControls();
	}

	return false; /* unused */
}

bool PrFindOrientation::selectItem(IDispatch* control, long index, bool select)
{
	const int row = static_cast<int>(index & 0xFFFF);
	const int column = static_cast<int>(index >> 16);

	m_currentGridRow = m_resultGrid->CurrentRow;

	m_propertyManager->SpecToolbar = (m_currentGridRow != 0) ? kapi::SpecPropertyToolBarEnum::pnEnterEscHelp : kapi::SpecPropertyToolBarEnum::pnEscHelp;

	updateScene();

	return false;
}

void PrFindOrientation::initControls()
{
	{
		kapi::IPropertyGroupBeginPtr settingsGroupBegin = m_controls->Add(kapi::ControlTypeEnum::ksControlGroupBegin);
		settingsGroupBegin->Name = "Параметры";
		settingsGroupBegin->Expanding = true;

		m_ctrls.overhangThreshold = createSettingEdit(m_controls, si::overhangThreshold, kapi::ControlTypeEnum::ksControlEditInt, "Максимальный угол нависаний");

		m_ctrls.bottomThreshold = m_controls->Add(kapi::ControlTypeEnum::ksControlEditReal);
		m_ctrls.bottomThreshold->Name = "Погрешность нижней повехрности";

		m_ctrls.accuracy = m_controls->Add(kapi::ControlTypeEnum::ksControlListStr);
		m_ctrls.accuracy->Name = L"Точность";
		m_ctrls.accuracy->ReadOnly = true;
		for (size_t i = 0; i < enums::toUnderlying(Accuracy::count); ++i) {
			m_ctrls.accuracy->Add(c_accuracyNames.at(static_cast<Accuracy>(i)).data());
		}

		m_recalcButton = m_controls->Add(kapi::ControlTypeEnum::ksControlTextButton);
		m_recalcButton->Id = 1;
		m_recalcButton->Name = L"Рассчитать";

		m_controls->Add(kapi::ControlTypeEnum::ksControlGroupEnd); /*settings*/
	}
	{
		kapi::IPropertyGroupBeginPtr resultsGroupBegin = m_controls->Add(kapi::ControlTypeEnum::ksControlGroupBegin);
		resultsGroupBegin->Name = "Результаты";
		resultsGroupBegin->Expanding = true;

		m_metricsList = m_controls->Add(kapi::ControlTypeEnum::ksControlListStr);
		m_metricsList->Name = L"Метрика";
		m_metricsList->ReadOnly = true;
		for (size_t i = 0; i < enums::toUnderlying(OrientationComplexCriteria::count); ++i) {
			m_metricsList->Add(c_metricNames.at(static_cast<OrientationComplexCriteria>(i)).data());
		}

		m_visualizeCheckBox = m_controls->Add(kapi::ControlTypeEnum::ksControlCheckBox);
		m_visualizeCheckBox->Name = L"Показывать тепловую карту";

		m_ctrls.resultCount = m_controls->Add(kapi::ControlTypeEnum::ksControlEditReal);
		m_ctrls.resultCount->Name = "Кол-во вариантов ориентаций";

		m_resultGrid = m_controls->Add(kapi::ControlTypeEnum::ksControlGrid);
		m_resultGrid->Name = L"Результаты";
		m_resultGrid->ColumnCount = 6;
		m_resultGrid->CellText[0][0] = L"№";
		m_resultGrid->CellText[0][1] = L"S_o, мм2"; // Площадь нависаний
		m_resultGrid->CellText[0][2] = L"V_o, мм3"; // Объем поддержек
		m_resultGrid->CellText[0][3] = L"S_b, мм2"; // Площадь нижней поверхности
		m_resultGrid->CellText[0][4] = L"S_ch, мм2"; // Площадь выпуклого многоугольника нижней поверхности
		m_resultGrid->CellText[0][5] = L"H, мм"; // Высота модели

		m_controls->Add(kapi::ControlTypeEnum::ksControlGroupEnd); /*results*/
	}
}

void PrFindOrientation::updateControls()
{
	m_ctrls.overhangThreshold->Value = m_overhangThreshold;
	m_ctrls.bottomThreshold->Value = m_bottomThreshold;
	m_ctrls.accuracy->SetCurrentByIndex(enums::toUnderlying(m_accuracy));

	if (m_stat) {
		m_metricsList->Visible = true;
		m_visualizeCheckBox->Visible = true;
		m_ctrls.resultCount->Visible = true;
		m_resultGrid->Visible = true;

		m_metricsList->SetCurrentByIndex(enums::toUnderlying(m_criteria));
		m_visualizeCheckBox->Value = m_isShowHeatmap;
		m_ctrls.resultCount->Value = m_resultCount;
		m_orientationsInGrid = m_stat->findBest(m_criteria, m_resultCount);
		refillGrid(m_orientationsInGrid);
		updateHeatmap();
	} else {
		m_metricsList->Visible = false;
		m_visualizeCheckBox->Visible = false;
		m_ctrls.resultCount->Visible = false;
		m_resultGrid->Visible = false;
	}
}

void PrFindOrientation::refillGrid(std::span<const size_t> indexes)
{
	if (!m_stat) {
		assert(false);
		return;
	}

	auto toStr_d = [](double num) { return _bstr_t(std::format("{:.2f}", num).c_str()); };
	auto toStr_i = [](long num) { return _bstr_t(std::format("{}", num).c_str()); };

	m_resultGrid->RowCount = static_cast<long>(indexes.size() + 1);
	for (long i = 0; i < indexes.size(); ++i) {
		m_resultGrid->CellText[i + 1][0] = toStr_i(i + 1);
		m_resultGrid->CellText[i + 1][1] = toStr_d(m_stat->infos[indexes[i]].overhangArea);
		m_resultGrid->CellText[i + 1][2] = toStr_d(m_stat->infos[indexes[i]].overhangVolume);
		m_resultGrid->CellText[i + 1][3] = toStr_d(m_stat->infos[indexes[i]].bottomArea);
		m_resultGrid->CellText[i + 1][4] = toStr_d(m_stat->infos[indexes[i]].bottomConvexHullArea);
		m_resultGrid->CellText[i + 1][5] = toStr_d(m_stat->infos[indexes[i]].modelHeight);
	}

	m_resultGrid->CurrentRow = static_cast<long>(m_currentGridRow);

	m_resultGrid->UpdateParam();
}

void PrFindOrientation::updateHeatmap()
{
	if (!m_isShowHeatmap || !m_stat)
		return;

	std::vector<glm::vec3> colors(m_stat->evalMesh.normals.size(), glm::vec3());

	const auto red = color::getStandardColor<color::HSV, color::StandardColor::red>();
	const auto green = color::getStandardColor<color::HSV, color::StandardColor::green>();
	auto toHeatmap = std::bind(math::convertRanges, std::placeholders::_1, 0.0, 1.0, red.hue, green.hue - red.hue);

	// value [0, 1] -> HSV color
	auto toColor = [&toHeatmap](double value) -> glm::vec3
	{
		color::RGB rgb = color::toRGB(color::HSV{ toHeatmap(1.0 - value), 1.0, 1.0 });
		return glm::vec3(rgb.red, rgb.green, rgb.blue);
	};
	const auto & complexCriteriaValues = m_stat->complexInfos[enums::toUnderlying(m_criteria)];
	std::ranges::transform(complexCriteriaValues, colors.begin(), toColor);

	auto mesh = std::make_shared<ColoredMesh>();
	mesh->positions = m_stat->evalMesh.positions;
	mesh->normals = m_stat->evalMesh.normals;
	mesh->indexes = m_stat->evalMesh.indexes;
	mesh->colors = colors;

	// Масштабирование икосферы по габариту детали
	kapi::ksPartPtr part = m_documentData.getDocument()->GetPart(kapi::Part_Type::pTop_Part);
	kapi::ksBodyPtr body = checkPtr(part)->GetMainBody();
	const geometry::Gabarit3D gabarit = getGabarit(body);
	const geometry::Vector3D center = gabarit.center();
	const double radius = std::max(std::max(gabarit.x.length(), gabarit.y.length()), gabarit.z.length()) / 2.0;

	glm::mat4 matrix = glm::translate(glm::mat4(1.0f), glm::vec3(center.x, center.y, center.z));
	matrix = glm::scale(matrix, glm::vec3(radius, radius, radius));
	std::ranges::transform(mesh->positions, mesh->positions.begin(), [&matrix](const glm::vec3& pos)
		{
			glm::vec4 res = matrix * glm::vec4(pos, 1.0);
			return glm::vec3(res.x, res.y, res.z);
		});
	
	auto hm = m_documentData.getHighlightingManager();
	hm->addObject(mesh, Visualizer::colorMesh);

	if (m_currentGridRow != 0) {
		const glm::vec3 point = mesh->positions[m_orientationsInGrid[m_currentGridRow - 1]];
		const glm::vec3 normal = glm::normalize(mesh->normals[m_orientationsInGrid[m_currentGridRow - 1]]);
		const glm::vec3 point2 = point + (normal * static_cast<float>(radius * 0.2));

		auto line = std::make_shared<Polyline3D>();
		line->m_points = { point, point2 };
		hm->addObject(line, Visualizer::polyline);
	}
}

void PrFindOrientation::updateScene()
{
	auto hm = m_documentData.getHighlightingManager();
	hm->cleanObjects();

	if (m_currentGridRow != 0 && !m_isShowHeatmap) {
		m_stat->updateMeshColors(m_orientationsInGrid[m_currentGridRow - 1]);
		hm->addObject(m_stat->model, Visualizer::colorMesh);

		BottomContour contour = m_stat->infos[m_orientationsInGrid[m_currentGridRow - 1]].bottomContour;
		if (contour.size() >= 3) {
			auto polyline = std::make_shared<Polyline3D>();
			polyline->m_points = contour;

			hm->addObject(polyline, Visualizer::polyline);
		}
	}

	updateHeatmap();
}
