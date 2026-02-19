#pragma once

#include "kapiwrap/PropertyManagerObject.hpp"
#include "settings/DocumentData.hpp"

class PrFindOrientation : public PropertyManagerObject
{
public:
	PrFindOrientation(kapi::KompasObjectPtr kompas, DocumentData & documentData);

private:
	virtual bool buttonClick(long buttonId) override;
	virtual bool changeControlValue(IDispatch* control);

private:
	void initControls();

	DocumentData& m_documentData;

	kapi::IPropertyTabPtr m_mainTab;
	kapi::IPropertyControlsPtr m_controls;

	kapi::IPropertyListPtr m_metricsList;
	kapi::IPropertyCheckBoxPtr m_visualizeCheckBox;
};
