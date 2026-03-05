#include "PrFindOrientation.hpp"

#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "kapiwrap/3d/body.hpp"
#include "settings/SettingInitializer.hpp"
#include "generic/color.hpp"
#include "utils.hpp"

namespace
{
constexpr std::wstring_view c_commonMetricName = L"Общая";
constexpr std::wstring_view c_overhangsMetricName = L"Площадь нависаний";
constexpr std::wstring_view c_bottomAreaMetricName = L"Площадь нижней грани";
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

double convertRanges(double baseValue, double baseBegin, double baseLength, double resultBegin, double resultLength)
{
	const double k = resultLength / baseLength;
	const double basePos = baseValue - baseBegin;
	return resultBegin + (basePos * k);
}

bool PrFindOrientation::changeControlValue(IDispatch* control)
{
	if (static_cast<bool>(m_visualizeCheckBox->Value) && m_stat) {
		OrientationStatByMesh& stat = *m_stat;

		std::vector<glm::vec3> colors(stat.evalMesh.normals.size(), glm::vec3());

		const auto red = color::getStandardColor<color::HSV, color::StandardColor::red>();
		const auto green = color::getStandardColor<color::HSV, color::StandardColor::green>();
		auto toHeatmap = std::bind(convertRanges, std::placeholders::_1, 0.0, std::placeholders::_2, red.hue, green.hue - red.hue);

		if (static_cast<_bstr_t>(m_metricsList->Value) == _bstr_t(c_overhangsMetricName.data())) {

			const double maxArea = *std::max_element(stat.overhangsArea.begin(), stat.overhangsArea.end());
			auto toColor = [&toHeatmap, maxArea](double overhangArea) -> glm::vec3
				{
					color::RGB rgb = color::toRGB(color::HSV{ toHeatmap(maxArea - overhangArea, maxArea), 1.0, 1.0 });
					return glm::vec3(rgb.red, rgb.green, rgb.blue);
				};
			std::ranges::transform(stat.overhangsArea, colors.begin(), toColor);
		} else if (static_cast<_bstr_t>(m_metricsList->Value) == _bstr_t(c_bottomAreaMetricName.data())) {
			auto maxArea = *std::max_element(stat.printSurfacesArea.begin(), stat.printSurfacesArea.end());
			auto toColor = [&toHeatmap, maxArea](double psArea) -> glm::vec3
				{
					color::RGB rgb = color::toRGB(color::HSV{ toHeatmap(psArea, maxArea), 1.0, 1.0 });
					return glm::vec3(rgb.red, rgb.green, rgb.blue);
				};
			std::ranges::transform(stat.printSurfacesArea, colors.begin(), toColor);
		} else if (static_cast<_bstr_t>(m_metricsList->Value) == _bstr_t(c_commonMetricName.data())) {
			
			auto maxOverhang = *std::max_element(stat.overhangsArea.begin(), stat.overhangsArea.end());
			auto maxBottom = *std::max_element(stat.printSurfacesArea.begin(), stat.printSurfacesArea.end());

			for (size_t i = 0; i < stat.evalMesh.normals.size(); ++i) {
				const double overhangArea = stat.overhangsArea[i];
				const double bottomArea = stat.printSurfacesArea[i];

				double hue = toHeatmap((maxOverhang - overhangArea) + bottomArea, maxOverhang + maxBottom);
				color::RGB rgb = color::toRGB(color::HSV{ hue , 1.0, 1.0 });
				colors[i] = glm::vec3(rgb.red, rgb.green, rgb.blue);
			}
		}

		auto mesh = std::make_shared<ColoredMesh>();
		mesh->positions = stat.evalMesh.positions;
		mesh->normals = stat.evalMesh.normals;
		mesh->indexes = stat.evalMesh.indexes;
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
		hm->setCustomMesh(mesh);
	} else {
		auto hm = m_documentData.getHighlightingManager();
		hm->setCustomMesh(nullptr);
	}

	return false; /* unused */
}

bool PrFindOrientation::controlCommand(IDispatch* control, long buttonId)
{
	if (buttonId == m_recalcButton->Id) {
		kapi::ksPartPtr part = m_documentData.getDocument()->GetPart(kapi::Part_Type::pTop_Part);
		const double overhangThreshold = m_documentData.getSettings()->getDoubleSetting(si::overhangThreshold.name)->getValue();
		m_stat = std::make_unique<OrientationStatByMesh>(calcOrientationStatByMesh(part->GetMainBody(), overhangThreshold));
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

			m_metricsList->Add(c_commonMetricName.data());
			m_metricsList->Add(c_overhangsMetricName.data());
			m_metricsList->Add(c_bottomAreaMetricName.data());

			m_metricsList->SetCurrentByIndex(0);
		}
		{
			m_visualizeCheckBox = m_controls->Add(kapi::ControlTypeEnum::ksControlCheckBox);
			m_visualizeCheckBox->Name = L"Показывать визуализацию";
		}
		{
			m_recalcButton = m_controls->Add(kapi::ControlTypeEnum::ksControlTextButton);
			m_recalcButton->Id = 1;
			m_recalcButton->Name = L"Рассчитать";
		}

		m_controls->Add(kapi::ControlTypeEnum::ksControlGroupEnd);
	}
}
