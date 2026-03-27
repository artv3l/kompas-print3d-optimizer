#include "PrFindOrientation.hpp"

#include <algorithm>
#include <format>
#include <ranges>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "kapiwrap/3d/body.hpp"
#include "settings/SettingInitializer.hpp"
#include "generic/color.hpp"
#include "utils.hpp"
#include "settings/SettingsManager.hpp"

namespace
{
const std::unordered_map<OrientationComplexCriteria, std::wstring_view> c_metricNames = {
	{OrientationComplexCriteria::overhangs, L"Количество поддержек"},
	{OrientationComplexCriteria::bottomQuality, L"Нижняя поверхность"},
	{OrientationComplexCriteria::common, L"Общий"}
};
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

	initControls();
	updateControls();
}

bool PrFindOrientation::buttonClick(long buttonId)
{
	auto hm = m_documentData.getHighlightingManager();
	hm->cleanObjects();

	switch (buttonId) {
	case kapi::SpecPropertyButtonEnum::pbEnter:
		// TODO Синхронизация максимального угла нависаний с переменными
		hide();
		break;
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

	m_isShowHeatmap = static_cast<bool>(m_visualizeCheckBox->Value);

	m_overhangThreshold = m_ctrls.overhangThreshold->Value;
	m_bottomThreshold = m_ctrls.bottomThreshold->Value;

	updateControls();

	return false; /* unused */
}

bool PrFindOrientation::controlCommand(IDispatch* control, long buttonId)
{
	if (buttonId == m_recalcButton->Id) {
		kapi::ksPartPtr part = m_documentData.getDocument()->GetPart(kapi::Part_Type::pTop_Part);
		m_stat = std::make_unique<OrientationStatByMesh>(calcOrientationStatByMesh(part->GetMainBody(), m_overhangThreshold, m_bottomThreshold));

		updateControls();
	}

	return false; /* unused */
}

bool PrFindOrientation::selectItem(IDispatch* control, long index, bool select)
{
	const int row = static_cast<int>(index & 0xFFFF);
	const int column = static_cast<int>(index >> 16);

	m_selectedOrientation = (row == 0) ? std::nullopt : std::make_optional(m_orientationsInGrid[row - 1]);
	updateScene();

	return false;
}

void PrFindOrientation::initControls()
{
	{
		m_ctrls.overhangThreshold = createSettingEdit(m_controls, si::overhangThreshold, kapi::ControlTypeEnum::ksControlEditInt, "Максимальный угол нависаний");
	}
	{
		m_ctrls.bottomThreshold = m_controls->Add(kapi::ControlTypeEnum::ksControlEditReal);
		m_ctrls.bottomThreshold->Name = "Погрешность нижней повехрности";
	}
	{
		m_recalcButton = m_controls->Add(kapi::ControlTypeEnum::ksControlTextButton);
		m_recalcButton->Id = 1;
		m_recalcButton->Name = L"Рассчитать";
	}
	{
		m_metricsList = m_controls->Add(kapi::ControlTypeEnum::ksControlListStr);
		m_metricsList->Name = L"Метрика";
		m_metricsList->ReadOnly = true;

		for (size_t i = 0; i < enums::toUnderlying(OrientationComplexCriteria::count); ++i) {
			m_metricsList->Add(c_metricNames.at(static_cast<OrientationComplexCriteria>(i)).data());
		}
	}
	{
		m_visualizeCheckBox = m_controls->Add(kapi::ControlTypeEnum::ksControlCheckBox);
		m_visualizeCheckBox->Name = L"Показывать тепловую карту";
	}
	{
		m_resultGrid = m_controls->Add(kapi::ControlTypeEnum::ksControlGrid);
		m_resultGrid->Name = L"Результаты";
		m_resultGrid->ColumnCount = 6;
		m_resultGrid->CellText[0][0] = L"№";
		m_resultGrid->CellText[0][1] = L"Площадь нависаний";
		m_resultGrid->CellText[0][2] = L"Объем поддержек";
		m_resultGrid->CellText[0][3] = L"Площадь нижней поверхности";
		m_resultGrid->CellText[0][4] = L"Площадь выпуклого многоугольника нижней поверхности";
		m_resultGrid->CellText[0][5] = L"Высота модели";
	}
}

void PrFindOrientation::updateControls()
{
	m_ctrls.overhangThreshold->Value = m_overhangThreshold;
	m_ctrls.bottomThreshold->Value = m_bottomThreshold;

	if (m_stat) {
		m_metricsList->Visible = true;
		m_visualizeCheckBox->Visible = true;
		m_resultGrid->Visible = true;

		m_metricsList->SetCurrentByIndex(enums::toUnderlying(m_criteria));
		m_visualizeCheckBox->Value = m_isShowHeatmap;
		m_orientationsInGrid = m_stat->findBest(m_criteria, 5);
		refillGrid(m_orientationsInGrid);
		updateHeatmap();
	} else {
		m_metricsList->Visible = false;
		m_visualizeCheckBox->Visible = false;
		m_resultGrid->Visible = false;
	}
}

void PrFindOrientation::refillGrid(std::span<const size_t> indexes)
{
	if (!m_stat) {
		assert(false);
		return;
	}

	auto toStr = [](auto num)
	{
		return _bstr_t(std::format(L"{}", num).c_str());;
	};

	m_resultGrid->RowCount = indexes.size() + 1;
	for (size_t i = 0; i < indexes.size(); ++i) {
		m_resultGrid->CellText[i + 1][0] = toStr(i + 1);
		m_resultGrid->CellText[i + 1][1] = toStr(m_stat->infos[indexes[i]].overhangArea);
		m_resultGrid->CellText[i + 1][2] = toStr(m_stat->infos[indexes[i]].overhangVolume);
		m_resultGrid->CellText[i + 1][3] = toStr(m_stat->infos[indexes[i]].bottomArea);
		m_resultGrid->CellText[i + 1][4] = toStr(m_stat->infos[indexes[i]].bottomConvexHullArea);
		m_resultGrid->CellText[i + 1][5] = toStr(m_stat->infos[indexes[i]].modelHeight);
	}

	m_resultGrid->UpdateParam();
}

void PrFindOrientation::updateHeatmap()
{
	if (m_isShowHeatmap && m_stat) {

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

		if (m_criteria == OrientationComplexCriteria::overhangs) {
			auto overhangs = m_stat->complexInfos[enums::toUnderlying(OrientationComplexCriteria::overhangs)];
			std::ranges::transform(overhangs, colors.begin(), toColor);
		} else if (m_criteria == OrientationComplexCriteria::bottomQuality) {
			auto bottomAreas = m_stat->complexInfos[enums::toUnderlying(OrientationComplexCriteria::bottomQuality)];
			std::ranges::transform(bottomAreas, colors.begin(), toColor);
		} else if (m_criteria == OrientationComplexCriteria::common) {

		}

		auto mesh = std::make_shared<ColoredMesh>();
		mesh->positions = m_stat->evalMesh.positions;
		mesh->normals = m_stat->evalMesh.normals;
		mesh->indexes = m_stat->evalMesh.indexes;
		mesh->colors = colors;

		{
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
		}

		auto hm = m_documentData.getHighlightingManager();
		hm->addObject(mesh, Visualizer::colorMesh);
	}
}

void PrFindOrientation::updateScene()
{
	auto hm = m_documentData.getHighlightingManager();
	hm->cleanObjects();

	if (m_selectedOrientation && !m_isShowHeatmap) {
		hm->addObject(m_stat->model, Visualizer::grayMesh);

		BottomContour contour = m_stat->infos[*m_selectedOrientation].bottomContour;
		if (contour.size() >= 3) {
			auto polyline = std::make_shared<geometry::Polyline3D>();
			polyline->m_points = contour;

			hm->addObject(polyline, Visualizer::polyline);
		}
	}

	updateHeatmap();
}
