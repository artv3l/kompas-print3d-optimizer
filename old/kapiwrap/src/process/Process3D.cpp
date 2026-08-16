#include "kapiwrap/process/Process3D.hpp"

Process3D::Process3D(ksapi::IApplication& kompasApp, ksapi::IKompasDocument3DPtr document, std::wstring_view eventsOwnerName, std::wstring_view caption):
	m_document(document),
	m_eventsOwnerName(eventsOwnerName),
	m_params(kompasApp.CreateProcessParam()),
	m_process3d(document->GetLibProcess(ksProcess3DTypeEnum::ksProcess3DSelectEntity)),
	m_process(m_process3d),
	m_paramEvents(m_params->Events())
{
	namespace stdph = std::placeholders;

	m_process->SetProcessParam(m_params);
	m_process->SetCaption(std::wstring(caption));

	m_paramEvents->AddChangeControlValueHandler(m_eventsOwnerName, std::bind(&Process3D::changeControlValue, this, stdph::_1));
	m_paramEvents->AddButtonClickHandler(m_eventsOwnerName, std::bind(&Process3D::buttonClick, this, stdph::_1));
	m_paramEvents->AddSelectItemHandler(m_eventsOwnerName, std::bind(&Process3D::selectItem, this, stdph::_1, stdph::_2, stdph::_3));
	m_paramEvents->AddControlCommandHandler(m_eventsOwnerName, std::bind(&Process3D::controlCommand, this, stdph::_1, stdph::_2));
}

Process3D::~Process3D()
{
	m_paramEvents->RemoveAllHandlers(m_eventsOwnerName);
}

void Process3D::run() const
{
	m_process->Run(true, true);
}
