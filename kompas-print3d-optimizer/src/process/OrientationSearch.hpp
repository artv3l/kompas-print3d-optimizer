#pragma once

#include <KsAPI.h>

#include "kapiwrap/process/Process3D.hpp"

class OrientationSearch final : public Process3D
{
public:
	OrientationSearch(ksapi::IApplication& kompasApp, ksapi::IKompasDocument3DPtr document, std::wstring_view eventsOwnerName);

protected:
	void changeControlValue(const ksapi::IPropertyControlPtr& control) const override;
	bool buttonClick(int32_t buttonId) const override;
	void selectItem(const ksapi::IPropertyControlPtr& control, int32_t index, bool select) const override;
	void controlCommand(const ksapi::IPropertyControlPtr& control, int32_t buttonId) const override;

private:
	void initControls();

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
};
