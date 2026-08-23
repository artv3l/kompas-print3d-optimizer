#pragma once

#include <span>

#include <KsAPI.h>

#include "kapiwrap/process/Process3D.hpp"
#include "core/orientation/orientation.hpp"
#include "settings/DocumentData.hpp"

enum class Accuracy : uint8_t
{
	low,
	medium,
	high,
	count
};

class OrientationSearch final : public Process3D
{
public:
	OrientationSearch(ksapi::IApplication& kompasApp, ksapi::IKompasDocument3DPtr document, std::wstring_view eventsOwnerName, DocumentData& documentData);

protected:
	void changeControlValue(const ksapi::IPropertyControlPtr& control) override;
	bool buttonClick(int32_t buttonId) override;
	void selectItem(const ksapi::IPropertyControlPtr& control, int32_t index, bool select) override;
	void controlCommand(const ksapi::IPropertyControlPtr& control, int32_t buttonId) override;

private:
	void initControls();
	void updateControls();
	void refillGrid();
	void updateScene();
	void updateToolbar();

private:
	struct
	{
		ksapi::IPropertyEditPtr overhangThreshold;
		ksapi::IPropertyEditPtr bottomThreshold;
		ksapi::IPropertyEditPtr resultCount;
		ksapi::IPropertyListPtr accuracy;
		ksapi::IPropertyTextButtonPtr recalcButton;
		ksapi::IPropertyListPtr metricsList;
		ksapi::IPropertyCheckBoxPtr visualizeCheckBox;
		ksapi::IPropertyGridPtr resultGrid;
	} m_ctrls;

	struct
	{
		OrientationComplexCriteria criteria = OrientationComplexCriteria::common;
		bool isShowHeatmap = false;
		int overhangThreshold = 45;
		double bottomThreshold = 0.2;
		size_t currentGridRow = 1; // 0 - РЎС‚СЂРѕРєР° РЅРµ РІС‹Р±СЂР°РЅР°
		std::vector<size_t> orientationsInGrid = {};
		size_t resultCount = 5; // РљРѕР»-РІРѕ РІР°СЂРёР°РЅС‚РѕРІ РѕСЂРёРµРЅС‚Р°С†РёР№ РґР»СЏ РІС‹РІРѕРґР° РІ С‚Р°Р±Р»РёС†Сѓ
		Accuracy accuracy = Accuracy::medium;
	} m_data;

	std::unique_ptr<OrientationStatByMesh> m_stat;

	DocumentData& m_documentData;
};
