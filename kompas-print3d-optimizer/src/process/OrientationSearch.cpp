#include "OrientationSearch.hpp"

#include <format>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "generic/enums.hpp"
#include "generic/math.hpp"
#include "core/orientation/orientation.hpp"
#include "kapiwrap/3d/part.hpp"

namespace
{
const std::unordered_map<Accuracy, std::wstring_view> c_accuracyNames = {
	{Accuracy::low, L"Низкая"},
	{Accuracy::medium, L"Средняя"},
	{Accuracy::high, L"Высокая"}
};

const std::unordered_map<OrientationComplexCriteria, std::wstring_view> c_metricNames = {
	{OrientationComplexCriteria::overhangs, L"Количество поддержек"},
	{OrientationComplexCriteria::bottomQuality, L"Нижняя поверхность"},
	{OrientationComplexCriteria::common, L"Общий"}
};

uint8_t getSubdivisionsCount(Accuracy accuracy)
{
	return enums::toUnderlying(accuracy) + 1;
}
}

OrientationSearch::OrientationSearch(ksapi::IApplication& kompasApp, ksapi::IKompasDocument3DPtr document, std::wstring_view eventsOwnerName, DocumentData& documentData):
	Process3D(kompasApp, document, eventsOwnerName, L"Поиск плоскости печати"),
	m_documentData(documentData)
{
	m_data.overhangThreshold = 45;
	m_data.bottomThreshold = 0.2;
	m_data.resultCount = 5;
	m_data.accuracy = Accuracy::medium;

	initControls();
	updateControls();
}

void OrientationSearch::changeControlValue(const ksapi::IPropertyControlPtr& control)
{
	for (size_t i = 0; i < enums::toUnderlying(OrientationComplexCriteria::count); ++i) {
		if (m_ctrls.metricsList->GetStringValue() == c_metricNames.at(static_cast<OrientationComplexCriteria>(i))) {
			m_data.criteria = static_cast<OrientationComplexCriteria>(i);
			break;
		}
	}
	for (size_t i = 0; i < enums::toUnderlying(Accuracy::count); ++i) {
		if (m_ctrls.accuracy->GetStringValue() == c_accuracyNames.at(static_cast<Accuracy>(i))) {
			m_data.accuracy = static_cast<Accuracy>(i);
			break;
		}
	}

	m_data.isShowHeatmap = m_ctrls.visualizeCheckBox->GetBoolValue();

	m_data.overhangThreshold = m_ctrls.overhangThreshold->GetIntValue();
	m_data.bottomThreshold = m_ctrls.bottomThreshold->GetDoubleValue();
	m_data.resultCount = m_ctrls.resultCount->GetIntValue();

	updateControls();
}

bool OrientationSearch::buttonClick(int32_t buttonId)
{
	DrawingManager& drawingManager = m_documentData.getDrawingManager();
	drawingManager.cleanObjects();

	switch (buttonId) {
	case SpecPropertyButtonEnum::pbEnter:
	{
		// Создание ЛСК плоскости печати
		const geom3d::Vec3 normal = m_stat->evalMesh.normals[m_data.orientationsInGrid[m_data.currentGridRow - 1]];
		const geom3d::Vec3 point = m_stat->infos[m_data.orientationsInGrid[m_data.currentGridRow - 1]].bottomContour[0];
		const geom3d::Plane plane(normal, point);
		
		//kapi::ksPartPtr part = m_documentData.getDocument()->GetPart(kapi::Part_Type::pTop_Part);
		//createLocalCS(part, plane);

		break;
	}
	}
	return true;
}

void OrientationSearch::selectItem(const ksapi::IPropertyControlPtr& control, int32_t index, bool select)
{
	const int row = static_cast<int>(index & 0xFFFF);
	const int column = static_cast<int>(index >> 16);

	m_data.currentGridRow = m_ctrls.resultGrid->GetCurrentRow();

	SpecPropertyToolBarEnum toolBar = (m_data.currentGridRow != 0) ? SpecPropertyToolBarEnum::pnEnterEscHelp : SpecPropertyToolBarEnum::pnEscHelp;
	m_params->SetSpecToolbar(toolBar);

	updateScene();
}

void OrientationSearch::controlCommand(const ksapi::IPropertyControlPtr& control, int32_t buttonId)
{
	if (buttonId == m_ctrls.recalcButton->GetId()) {
		ksapi::IKompasDocument3DPtr doc3d = m_documentData.getDoc();
		ksapi::IPartPtr part = doc3d->GetTopPart();
		
		m_stat = std::make_unique<OrientationStatByMesh>(calcOrientationStatByMesh(copyToMesh(part),
			m_data.overhangThreshold, m_data.bottomThreshold, getSubdivisionsCount(m_data.accuracy)
		));

		updateControls();
	}
}

void OrientationSearch::initControls()
{
	m_params->SetSpecToolbar(SpecPropertyToolBarEnum::pnEscHelp);

	ksapi::IPropertyControlsPtr controls = m_params->GetPropertyTabs()->Add(L"MainTab")->GetPropertyControls();

	{
		ksapi::IPropertyGroupBeginPtr settingsGroupBegin = controls->Add(ControlTypeEnum::ksControlGroupBegin);
		settingsGroupBegin->SetName(L"Параметры");
		settingsGroupBegin->SetExpanding(true);

		m_ctrls.overhangThreshold = controls->Add(ControlTypeEnum::ksControlEditInt);
		m_ctrls.overhangThreshold->SetName(L"Максимальный угол нависаний");

		m_ctrls.bottomThreshold = controls->Add(ControlTypeEnum::ksControlEditReal);
		m_ctrls.bottomThreshold->SetName(L"Погрешность нижней повехрности");

		m_ctrls.accuracy = controls->Add(ControlTypeEnum::ksControlListStr);
		m_ctrls.accuracy->SetName(L"Точность");
		m_ctrls.accuracy->SetReadOnly(true);

		std::vector<std::wstring> temp;
		for (size_t i = 0; i < enums::toUnderlying(Accuracy::count); ++i) {
			temp.push_back(c_accuracyNames.at(static_cast<Accuracy>(i)).data());
		}
		m_ctrls.accuracy->AddStringValues(temp);

		m_ctrls.recalcButton = controls->Add(ControlTypeEnum::ksControlTextButton);
		m_ctrls.recalcButton->SetId(1);
		m_ctrls.recalcButton->SetName(L"Рассчитать");

		controls->Add(ControlTypeEnum::ksControlGroupEnd);
	}
	{
		ksapi::IPropertyGroupBeginPtr resultsGroupBegin = controls->Add(ControlTypeEnum::ksControlGroupBegin);
		resultsGroupBegin->SetName(L"Результаты");
		resultsGroupBegin->SetExpanding(true);

		m_ctrls.metricsList = controls->Add(ControlTypeEnum::ksControlListStr);
		m_ctrls.metricsList->SetName(L"Метрика");
		m_ctrls.metricsList->SetReadOnly(true);

		std::vector<std::wstring> temp;
		for (size_t i = 0; i < enums::toUnderlying(OrientationComplexCriteria::count); ++i) {
			temp.push_back(c_metricNames.at(static_cast<OrientationComplexCriteria>(i)).data());
		}
		m_ctrls.metricsList->AddStringValues(temp);

		m_ctrls.visualizeCheckBox = controls->Add(ControlTypeEnum::ksControlCheckBox);
		m_ctrls.visualizeCheckBox->SetName(L"Показывать тепловую карту");

		m_ctrls.resultCount = controls->Add(ControlTypeEnum::ksControlEditInt);
		m_ctrls.resultCount->SetName(L"Кол-во вариантов ориентаций");

		m_ctrls.resultGrid = controls->Add(ControlTypeEnum::ksControlGrid);
		m_ctrls.resultGrid->SetName(L"Результаты");
		m_ctrls.resultGrid->SetColumnCount(5);
		m_ctrls.resultGrid->SetCellText(0, 0, L"№");
		m_ctrls.resultGrid->SetCellText(0, 1, L"S_o, мм2"); // Площадь нависаний
		m_ctrls.resultGrid->SetCellText(0, 2, L"S_b, мм2"); // Площадь нижней поверхности
		m_ctrls.resultGrid->SetCellText(0, 3, L"S_ch, мм2"); // Площадь выпуклого многоугольника нижней поверхности
		m_ctrls.resultGrid->SetCellText(0, 4, L"H, мм"); // Высота модели

		controls->Add(ControlTypeEnum::ksControlGroupEnd);
	}
}

void OrientationSearch::updateControls()
{
	m_ctrls.overhangThreshold->SetIntValue(m_data.overhangThreshold);
	m_ctrls.bottomThreshold->SetDoubleValue(m_data.bottomThreshold);
	m_ctrls.accuracy->SetCurrentByIndex(static_cast<int32_t>(enums::toUnderlying(m_data.accuracy)));
	m_ctrls.resultCount->SetIntValue(static_cast<int32_t>(m_data.resultCount));

	if (m_stat) {
		m_ctrls.metricsList->SetVisible(true);
		m_ctrls.visualizeCheckBox->SetVisible(true);
		m_ctrls.resultCount->SetVisible(true);
		m_ctrls.resultGrid->SetVisible(true);

		m_ctrls.metricsList->SetCurrentByIndex(static_cast<int32_t>(enums::toUnderlying(m_data.criteria)));
		m_ctrls.visualizeCheckBox->SetBoolValue(m_data.isShowHeatmap);
		m_data.orientationsInGrid = m_stat->findBest(m_data.criteria, m_data.resultCount);
		refillGrid(m_data.orientationsInGrid);
		updateHeatmap();
	}
	else {
		m_ctrls.metricsList->SetVisible(false);
		m_ctrls.visualizeCheckBox->SetVisible(false);
		m_ctrls.resultCount->SetVisible(false);
		m_ctrls.resultGrid->SetVisible(false);
	}
}

void OrientationSearch::refillGrid(std::span<const size_t> indexes)
{
	if (!m_stat) {
		assert(false);
		return;
	}

	auto toStr_d = [](double num) { return (std::format(L"{:.2f}", num)); };
	auto toStr_i = [](long num) { return (std::format(L"{}", num)); };

	m_ctrls.resultGrid->SetRowCount(static_cast<long>(indexes.size() + 1));
	for (long i = 0; i < indexes.size(); ++i) {
		m_ctrls.resultGrid->SetCellText(i + 1, 0, toStr_i(i + 1));
		m_ctrls.resultGrid->SetCellText(i + 1, 1, toStr_d(m_stat->infos[indexes[i]].overhangArea));
		m_ctrls.resultGrid->SetCellText(i + 1, 2, toStr_d(m_stat->infos[indexes[i]].bottomArea));
		m_ctrls.resultGrid->SetCellText(i + 1, 3, toStr_d(m_stat->infos[indexes[i]].bottomConvexHullArea));
		m_ctrls.resultGrid->SetCellText(i + 1, 4, toStr_d(m_stat->infos[indexes[i]].modelHeight));
	}

	m_ctrls.resultGrid->SetCurrentRow(static_cast<long>(m_data.currentGridRow));

	m_ctrls.resultGrid->UpdateParam();
}

void OrientationSearch::updateHeatmap()
{
	if (!m_data.isShowHeatmap || !m_stat)
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
	const auto& complexCriteriaValues = m_stat->complexInfos[enums::toUnderlying(m_data.criteria)];
	std::ranges::transform(complexCriteriaValues, colors.begin(), toColor);

	auto mesh = std::make_shared<ColoredMesh>(m_stat->evalMesh, orientation::c_defaultColor);
	mesh->colors = colors;

	// Масштабирование икосферы по габариту детали
	ksapi::IKompasDocument3DPtr doc3d = m_documentData.getDoc();
	ksapi::IPartPtr part = doc3d->GetTopPart();

	const geom3d::Gabarit gabarit = getGabarit(part);
	const Eigen::Vector3d center = gabarit.center();
	const double radius = (gabarit.getEnd() - gabarit.getBegin()).norm() / 2.0;

	glm::mat4 matrix = glm::translate(glm::mat4(1.0f), glm::vec3(center.x(), center.y(), center.z()));
	matrix = glm::scale(matrix, glm::vec3(radius, radius, radius));
	std::ranges::transform(mesh->positions, mesh->positions.begin(), [&matrix](const glm::vec3& pos)
		{
			glm::vec4 res = matrix * glm::vec4(pos, 1.0);
			return glm::vec3(res.x, res.y, res.z);
		});

	DrawingManager& drawingManager = m_documentData.getDrawingManager();
	drawingManager.addObject(mesh, Visualizer::colorMesh);

	if (m_data.currentGridRow != 0) {
		const glm::vec3 point = mesh->positions[m_data.orientationsInGrid[m_data.currentGridRow - 1]];
		const glm::vec3 normal = glm::normalize(mesh->normals[m_data.orientationsInGrid[m_data.currentGridRow - 1]]);
		const glm::vec3 point2 = point + (normal * static_cast<float>(radius * 0.2));

		auto points = { geom3d::Vec3(point.x, point.y, point.z), geom3d::Vec3(point2.x, point2.y, point2.z) };
		auto line = std::make_shared<Polyline3D>(points);
		drawingManager.addObject(line, Visualizer::polyline);
	}
}

void OrientationSearch::updateScene()
{
	DrawingManager& drawingManager = m_documentData.getDrawingManager();
	drawingManager.cleanObjects();

	if (m_data.currentGridRow != 0 && !m_data.isShowHeatmap) {
		m_stat->updateMeshColors(m_data.orientationsInGrid[m_data.currentGridRow - 1]);
		auto mesh = std::make_shared<ColoredMesh>(m_stat->model, orientation::c_defaultColor);

		mesh->colors.reserve(m_stat->colors.size());
		for (size_t i = 0; i < m_stat->colors.size(); ++i)
		{
			auto&& color = m_stat->colors[i];
			mesh->colors[i] = (glm::vec3(color.red, color.green, color.blue));
		}

		drawingManager.addObject(mesh, Visualizer::colorMesh);

		BottomContour contour = m_stat->infos[m_data.orientationsInGrid[m_data.currentGridRow - 1]].bottomContour;
		if (contour.size() >= 3) {
			auto polyline = std::make_shared<Polyline3D>(contour);
			drawingManager.addObject(polyline, Visualizer::polyline);
		}
	}

	updateHeatmap();
}
