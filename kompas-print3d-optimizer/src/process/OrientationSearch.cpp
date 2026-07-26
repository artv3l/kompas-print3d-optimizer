#include "OrientationSearch.hpp"

#include <format>

#include "generic/enums.hpp"
#include "core/orientation/orientation.hpp"

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
}

OrientationSearch::OrientationSearch(ksapi::IApplication& kompasApp, ksapi::IKompasDocument3DPtr document, std::wstring_view eventsOwnerName):
	Process3D(kompasApp, document, eventsOwnerName, L"Поиск плоскости печати")
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
	/*auto hm = m_documentData.getHighlightingManager();
	hm->cleanObjects();*/

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
		/*kapi::ksPartPtr part = m_documentData.getDocument()->GetPart(kapi::Part_Type::pTop_Part);
		kapi::ksBodyPtr body = part->GetMainBody();
		m_stat = std::make_unique<OrientationStatByMesh>(calcOrientationStatByMesh(copyToMesh(body),
			m_overhangThreshold, m_bottomThreshold, getSubdivisionsCount(m_accuracy)
		));*/

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
	m_ctrls.resultCount->SetIntValue(m_data.resultCount);

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
}

void OrientationSearch::updateScene()
{
}
