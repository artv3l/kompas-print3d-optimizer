#include "PrFindOrientation.hpp"

#include <algorithm>

#include "settings/SettingInitializer.hpp"

PrFindOrientation::PrFindOrientation(kapi::KompasObjectPtr kompas, DocumentData& documentData):
	PropertyManagerObject(kompas),
	m_documentData(documentData),
	m_mainTab(m_propertyManager->PropertyTabs->Add("MainTab")),
	m_controls(m_mainTab->PropertyControls)
{
	m_propertyManager->Layout = kapi::PropertyManagerLayout::pmAlignRight;
	m_propertyManager->SpecToolbar = kapi::SpecPropertyToolBarEnum::pnEnterEscHelp;
	m_propertyManager->Caption = L"Поиск плоскости печати";

	initControls();
}

bool PrFindOrientation::buttonClick(long buttonId)
{
	auto hm = m_documentData.getHighlightingManager();
	hm->setCustomMesh(nullptr);

	switch (buttonId) {
	case kapi::SpecPropertyButtonEnum::pbEnter:
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
	if (static_cast<bool>(m_visualizeCheckBox->Value)) {
		kapi::ksPartPtr part = m_documentData.getDocument()->GetPart(kapi::Part_Type::pTop_Part);
		const double overhangThreshold = m_documentData.getSettings()->getDoubleSetting(si::overhangThreshold.name)->getValue();
		OrientationStatByMesh stat = calcOrientationStatByMesh(part->GetMainBody(), overhangThreshold);

		std::vector<glm::vec3> colors(stat.evalMesh.normals.size(), glm::vec3());

		if (static_cast<_bstr_t>(m_metricsList->Value) == _bstr_t(L"Площадь нависаний")) { // TODO
			auto toColor = [bodyArea = stat.bodyArea](double overhangArea) -> glm::vec3
				{
					float v = 1.0f - static_cast<float>(overhangArea / bodyArea);
					return glm::vec3(1.0f - v, v, 0.0f);
				};
			std::ranges::transform(stat.overhangsArea, colors.begin(), toColor);
		} else if (static_cast<_bstr_t>(m_metricsList->Value) == _bstr_t(L"Площадь нижней грани")) { // TODO
			auto maxArea = *std::max_element(stat.printSurfacesArea.begin(), stat.printSurfacesArea.end());
			auto toColor = [maxArea](double psArea) -> glm::vec3
				{
					float v = 1.0f - static_cast<float>(psArea / maxArea);
					return glm::vec3(1.0f - v, v, 0.0f);
				};
			std::ranges::transform(stat.printSurfacesArea, colors.begin(), toColor);
		} else {
			assert(false);
		}

		auto mesh = std::make_shared<ColoredMesh>();
		mesh->positions = stat.evalMesh.positions;
		mesh->normals = stat.evalMesh.normals;
		mesh->indexes = stat.evalMesh.indexes;
		mesh->colors = colors;
		
		auto hm = m_documentData.getHighlightingManager();
		hm->setCustomMesh(mesh);
	} else {
		auto hm = m_documentData.getHighlightingManager();
		hm->setCustomMesh(nullptr);
	}

	return false; /* unused */
}

void PrFindOrientation::initControls()
{
	{
		kapi::IPropertyGroupBeginPtr visualizationGroupBegin = m_controls->Add(kapi::ControlTypeEnum::ksControlGroupBegin);
		visualizationGroupBegin->Name = "Визуализация";
		visualizationGroupBegin->Expanding = true;

		{
			m_metricsList = m_controls->Add(kapi::ControlTypeEnum::ksControlListStr);
			m_metricsList->Name = L"Метрика";
			m_metricsList->ReadOnly = true;

			m_metricsList->Add(L"Площадь нависаний");
			m_metricsList->Add(L"Площадь нижней грани");

			m_metricsList->SetCurrentByIndex(0);
		}
		{
			m_visualizeCheckBox = m_controls->Add(kapi::ControlTypeEnum::ksControlCheckBox);
			m_visualizeCheckBox->Name = L"Показывать визуализацию";
		}

		m_controls->Add(kapi::ControlTypeEnum::ksControlGroupEnd);
	}
}
