#pragma once

#include <KsAPI.h>

class Process3D
{
public:
	Process3D(ksapi::IApplication& kompasApp, ksapi::IKompasDocument3DPtr document, std::wstring_view eventsOwnerName);
	~Process3D();

	void run() const;

protected:
	virtual void changeControlValue(const ksapi::IPropertyControlPtr& control) const = 0;
	virtual bool buttonClick(int32_t buttonId) const = 0;
	virtual void selectItem(const ksapi::IPropertyControlPtr& control, int32_t index, bool select) const = 0;
	virtual void controlCommand(const ksapi::IPropertyControlPtr& control, int32_t buttonId) const = 0;

protected:
	ksapi::IProcessParamPtr m_params;

private:
	ksapi::IKompasDocumentPtr m_document;
	const std::wstring m_eventsOwnerName;

	ksapi::IProcess3DPtr m_process3d;
	ksapi::IProcessPtr m_process;

	ksapi::IPropertiesManagerEventsPtr m_paramEvents;
};
