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
	if (m_data.currentGridRow > m_data.resultCount) {
		m_data.currentGridRow = 1;
	}

	updateControls();
}

bool OrientationSearch::buttonClick(int32_t buttonId)
{
	DrawingManager& drawingManager = m_documentData.getDrawingManager();
	drawingManager.cleanObjects();
	drawingManager.redraw();

	switch (buttonId) {
	case SpecPropertyButtonEnum::pbEnter:
	{
		// Создание ЛСК плоскости печати
		const geom3d::Vec3 normal = m_stat->evalMesh.normals[m_data.orientationsInGrid[m_data.currentGridRow - 1]];
		const geom3d::Vec3 point = m_stat->infos[m_data.orientationsInGrid[m_data.currentGridRow - 1]].bottomContour[0];
		const geom3d::Plane plane(normal, point);

		break;
	}
	}
	return true;
}

void OrientationSearch::selectItem(const ksapi::IPropertyControlPtr& control, int32_t index, bool select)
{
	m_data.currentGridRow = std::clamp(static_cast<size_t>(m_ctrls.resultGrid->GetCurrentRow()), static_cast<size_t>(0), m_data.resultCount);

	updateScene();
	updateToolbar();
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

	if (m_stat) {
		m_ctrls.metricsList->SetVisible(true);
		m_ctrls.visualizeCheckBox->SetVisible(true);

		m_ctrls.resultCount->SetVisible(true);
		m_ctrls.resultCount->SetIntValue(static_cast<int32_t>(m_data.resultCount));
		m_ctrls.resultCount->SetValueRange(1, m_stat->evalMesh.normals.size());

		m_ctrls.resultGrid->SetVisible(true);

		m_ctrls.metricsList->SetCurrentByIndex(static_cast<int32_t>(enums::toUnderlying(m_data.criteria)));
		m_ctrls.visualizeCheckBox->SetBoolValue(m_data.isShowHeatmap);
		m_data.orientationsInGrid = m_stat->findBest(m_data.criteria, m_data.resultCount);
		refillGrid();
		updateScene();
	}
	else {
		m_ctrls.metricsList->SetVisible(false);
		m_ctrls.visualizeCheckBox->SetVisible(false);
		m_ctrls.resultCount->SetVisible(false);
		m_ctrls.resultGrid->SetVisible(false);
	}

	updateToolbar();
}

void OrientationSearch::refillGrid()
{
	if (!m_stat) {
		assert(false);
		return;
	}

	auto toStr_d = [](double num) { return (std::format(L"{:.2f}", num)); };
	auto toStr_i = [](long num) { return (std::format(L"{}", num)); };

	assert(m_data.resultCount == m_data.orientationsInGrid.size());
	m_ctrls.resultGrid->SetRowCount(m_data.resultCount + 1);
	for (long i = 0; i < m_data.orientationsInGrid.size(); ++i) {
		m_ctrls.resultGrid->SetCellText(i + 1, 0, toStr_i(i + 1));
		m_ctrls.resultGrid->SetCellText(i + 1, 1, toStr_d(m_stat->infos[m_data.orientationsInGrid[i]].overhangArea));
		m_ctrls.resultGrid->SetCellText(i + 1, 2, toStr_d(m_stat->infos[m_data.orientationsInGrid[i]].bottomArea));
		m_ctrls.resultGrid->SetCellText(i + 1, 3, toStr_d(m_stat->infos[m_data.orientationsInGrid[i]].bottomConvexHullArea));
		m_ctrls.resultGrid->SetCellText(i + 1, 4, toStr_d(m_stat->infos[m_data.orientationsInGrid[i]].modelHeight));
	}

	m_ctrls.resultGrid->SetCurrentRow(m_data.currentGridRow);

	m_ctrls.resultGrid->UpdateParam();
}

namespace
{
// Создать объект-прямоугольник для визуализации габарита в определенной СК
std::shared_ptr<IObject> createGabaritVisualizer(geom3d::Gabarit gabarit, const geom3d::Placement& placement)
{
	using Crnr = geom3d::Gabarit::CornerType;

	// Коэффициент расширения габарита относительно самой длинной стороны основания
	constexpr double c_enlargeCoef = 0.01;

	if (gabarit.isEmpty())
		return nullptr;

	const double maxSide = std::max(
		(gabarit.corner(Crnr::BottomLeftCeil) - gabarit.corner(Crnr::BottomRightCeil)).norm(),
		(gabarit.corner(Crnr::BottomRightCeil) - gabarit.corner(Crnr::TopRightCeil)).norm()
	);
	gabarit.min() -= geom3d::Vec3::Constant(maxSide * c_enlargeCoef);
	gabarit.max() += geom3d::Vec3::Constant(maxSide * c_enlargeCoef);

	const auto toWorld = placement.matrixToWorld();
	auto points = {
		toWorld * gabarit.corner(Crnr::BottomLeftCeil),
		toWorld * gabarit.corner(Crnr::BottomRightCeil),
		toWorld * gabarit.corner(Crnr::TopRightCeil),
		toWorld * gabarit.corner(Crnr::TopLeftCeil),
		toWorld * gabarit.corner(Crnr::BottomLeftCeil),
	};
	return std::make_shared<Polyline3D>(points, color::getStandardColor<color::RGB, color::StandardColor::blue>());
}
}

void OrientationSearch::updateScene()
{
	if (!m_stat)
		return;

	const std::optional<size_t> currentOrientationIndex = m_data.currentGridRow != 0 ?
		std::make_optional(m_data.orientationsInGrid[m_data.currentGridRow - 1]) : std::nullopt;
	const std::optional<geom3d::Placement> orientationPlacement = currentOrientationIndex ? 
		std::make_optional(geom3d::Placement::createByAxisZ(
			m_stat->evalMesh.positions[*currentOrientationIndex],
			m_stat->evalMesh.normals[*currentOrientationIndex])
		) : std::nullopt;

	DrawingManager& drawingManager = m_documentData.getDrawingManager();
	drawingManager.cleanObjects();

	if (!m_data.isShowHeatmap) {
		if (currentOrientationIndex && orientationPlacement) {
			m_stat->updateMeshColors(*currentOrientationIndex);
			auto mesh = std::make_shared<ColoredMesh>(m_stat->model, orientation::c_defaultColor, ColoredMesh::ColorType::byTriangle);

			mesh->colors.reserve(m_stat->colors.size());
			for (size_t i = 0; i < m_stat->colors.size(); ++i)
			{
				auto&& color = m_stat->colors[i];
				mesh->colors[i] = (glm::vec4(color.red, color.green, color.blue, 1.0f));
			}

			drawingManager.addObject(mesh, Visualizer::colorMesh);

			BottomContour contour = m_stat->infos[*currentOrientationIndex].bottomContour;
			if (contour.size() >= 3) {
				auto polyline = std::make_shared<Polyline3D>(contour, color::getStandardColor<color::RGB, color::StandardColor::green>());
				drawingManager.addObject(polyline, Visualizer::polyline);
			}

			// Прямоугольник-габарит нижней поверхности детали. Обозначает стол 3D-принтера
			const geom3d::Gabarit modelGabarit = geom3d::calcGabarit(m_stat->model, *orientationPlacement);
			if (auto gabaritVisualizer = createGabaritVisualizer(modelGabarit, *orientationPlacement))
				drawingManager.addObject(gabaritVisualizer, Visualizer::polyline);
		}
	} else {
		std::vector<glm::vec4> colors(m_stat->evalMesh.normals.size(), glm::vec4());

		const auto red = color::getStandardColor<color::HSV, color::StandardColor::red>();
		const auto green = color::getStandardColor<color::HSV, color::StandardColor::green>();
		auto toHeatmap = std::bind(math::convertRanges, std::placeholders::_1, 0.0, 1.0, red.hue, green.hue - red.hue);

		// value [0, 1] -> HSV color
		auto toColor = [&toHeatmap](double value) -> glm::vec4
			{
				color::RGB rgb = color::toRGB(color::HSV{ toHeatmap(1.0 - value), 1.0, 1.0 });
				return glm::vec4(rgb.red, rgb.green, rgb.blue, 1.0f);
			};
		const auto& complexCriteriaValues = m_stat->complexInfos[enums::toUnderlying(m_data.criteria)];
		std::ranges::transform(complexCriteriaValues, colors.begin(), toColor);

		auto mesh = std::make_shared<ColoredMesh>(m_stat->evalMesh, orientation::c_defaultColor, ColoredMesh::ColorType::byVertex);
		mesh->colors = colors;

		// Масштабирование икосферы по габариту детали
		ksapi::IKompasDocument3DPtr doc3d = m_documentData.getDoc();
		ksapi::IPartPtr part = doc3d->GetTopPart();

		const geom3d::Gabarit gabarit = getGabarit(part);
		const Eigen::Vector3d center = gabarit.center();
		const double radius = (gabarit.max() - gabarit.min()).norm() / 2.0;

		glm::mat4 matrix = glm::translate(glm::mat4(1.0f), glm::vec3(center.x(), center.y(), center.z()));
		matrix = glm::scale(matrix, glm::vec3(radius, radius, radius));
		std::ranges::transform(mesh->positions, mesh->positions.begin(), [&matrix](const glm::vec3& pos)
			{
				glm::vec4 res = matrix * glm::vec4(pos, 1.0);
				return glm::vec3(res.x, res.y, res.z);
			});

		drawingManager.addObject(mesh, Visualizer::smoothMesh);

		if (currentOrientationIndex && orientationPlacement) {
			const glm::vec3 point = mesh->positions[*currentOrientationIndex];
			const glm::vec3 normal = glm::normalize(mesh->normals[*currentOrientationIndex]);
			const glm::vec3 point2 = point + (normal * static_cast<float>(radius * 0.2));

			auto points = { geom3d::Vec3(point.x, point.y, point.z), geom3d::Vec3(point2.x, point2.y, point2.z) };
			auto line = std::make_shared<Polyline3D>(points, color::getStandardColor<color::RGB, color::StandardColor::blue>());
			drawingManager.addObject(line, Visualizer::polyline);

			// Прямоугольник-габарит нижней поверхности детали. Обозначает стол 3D-принтера
			const geom3d::Gabarit sphereGabarit(-geom3d::Vec3::Constant(radius), geom3d::Vec3::Constant(radius));
			if (auto gabaritVisualizer = createGabaritVisualizer(sphereGabarit, *orientationPlacement))
				drawingManager.addObject(gabaritVisualizer, Visualizer::polyline);
		}
	}

	drawingManager.redraw();
}

void OrientationSearch::updateToolbar()
{
	SpecPropertyToolBarEnum toolBar = m_stat ? SpecPropertyToolBarEnum::pnEnterEscHelp : SpecPropertyToolBarEnum::pnEscHelp;
	m_params->SetSpecToolbar(toolBar);
}
