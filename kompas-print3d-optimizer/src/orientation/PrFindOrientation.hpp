#pragma once

#include <optional>

#include "kapiwrap/PropertyManagerObject.hpp"
#include "settings/DocumentData.hpp"

class PrFindOrientation : public PropertyManagerObject
{
public:
	PrFindOrientation(kapi::KompasObjectPtr kompas, DocumentData & documentData);

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
	} m_ctrls;

	kapi::IPropertyListPtr m_metricsList;
	kapi::IPropertyCheckBoxPtr m_visualizeCheckBox;
	kapi::IPropertyTextButtonPtr m_recalcButton;
	kapi::IPropertyGridPtr m_resultGrid;
	
	OrientationComplexCriteria m_criteria = OrientationComplexCriteria::common;
	bool m_isShowHeatmap = false;
	double m_overhangThreshold = 0.0;
	double m_bottomThreshold = 0.0;
	size_t m_currentGridRow = 0; // 0 - СЃС‚СЂРѕРєР° РЅРµ РІС‹Р±СЂР°РЅР°
	std::vector<size_t> m_orientationsInGrid;

	std::unique_ptr<OrientationStatByMesh> m_stat;
};
