#include "OptionalControl.hpp"

template class OptionalControl<kapi::ControlTypeEnum::ksControlEditReal>;

template<kapi::ControlTypeEnum controlType>
inline OptionalControl<controlType>::OptionalControl(kapi::IPropertyControlsPtr controls, bool isEnableOnInit, const _bstr_t& checkBoxName)
{
    m_checkBox = controls->Add(kapi::ControlTypeEnum::ksControlCheckBox);
    m_control = controls->Add(controlType);

    m_checkBox->Name = L"test";
    m_checkBox->Value = true;

}
