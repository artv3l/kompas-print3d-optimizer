#include "OrientationSearch.hpp"

#include "generic/enums.hpp"
#include "core/orientation/orientation.hpp"

namespace
{
enum class Accuracy : uint8_t
{
	low,
	medium,
	high,
	count
};

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
	Process3D(kompasApp, document, eventsOwnerName)
{
	initControls();
}

void OrientationSearch::changeControlValue(const ksapi::IPropertyControlPtr& control) const
{
}

bool OrientationSearch::buttonClick(int32_t buttonId) const
{
	return false;
}

void OrientationSearch::selectItem(const ksapi::IPropertyControlPtr& control, int32_t index, bool select) const
{
}

void OrientationSearch::controlCommand(const ksapi::IPropertyControlPtr& control, int32_t buttonId) const
{
}

void OrientationSearch::initControls()
{
	m_params->SetSpecToolbar(SpecPropertyToolBarEnum::pnEnterEscHelp);

	ksapi::IPropertyControlsPtr controls = m_params->GetPropertyTabs()->Add(L"MainTab")->GetPropertyControls();

	{
		ksapi::IPropertyGroupBeginPtr settingsGroupBegin = controls->Add(ControlTypeEnum::ksControlGroupBegin);
		settingsGroupBegin->SetName(L"Параметры");
		settingsGroupBegin->SetExpanding(true);

		//m_ctrls.overhangThreshold = createSettingEdit(controls, si::overhangThreshold, kapi::ControlTypeEnum::ksControlEditInt, "Максимальный угол нависаний");

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

		m_ctrls.resultCount = controls->Add(ControlTypeEnum::ksControlEditReal);
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
