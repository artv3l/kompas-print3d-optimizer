#ifndef OPTIONAL_CONTROL_HPP
#define OPTIONAL_CONTROL_HPP

#include <type_traits>

namespace detail {

    template <kapi::ControlTypeEnum controlType>
    struct GetControlType;

    template <kapi::ControlTypeEnum controlType>
    using GetControlType_t = typename GetControlType<controlType>::Type;

    template <typename T>
    struct DeclareUsingType {
        using Type = T;
    };

    template <> struct GetControlType<kapi::ControlTypeEnum::ksControlCheckBox> : DeclareUsingType<kapi::IPropertyCheckBoxPtr> {};
    template <> struct GetControlType<kapi::ControlTypeEnum::ksControlEditReal> : DeclareUsingType<kapi::IPropertyEditPtr> {};
    template <> struct GetControlType<kapi::ControlTypeEnum::ksControlTwinSwitcher> : DeclareUsingType<kapi::IPropertyTwinSwitcherPtr> {};

}

template <kapi::ControlTypeEnum controlType>
class OptionalControl final {
public:
    OptionalControl(kapi::IPropertyControlsPtr controls, bool isEnableOnInit, const _bstr_t& checkBoxName);
    OptionalControl(const OptionalControl& obj) = delete;
    OptionalControl(OptionalControl&& obj) noexcept = delete;

public:
    OptionalControl& operator=(const OptionalControl& obj) = delete;
    OptionalControl& operator=(OptionalControl&& obj) noexcept = delete;

private:
    kapi::IPropertyCheckBoxPtr m_checkBox = nullptr;
    detail::GetControlType_t<controlType> m_control = nullptr;

};



#endif /* OPTIONAL_CONTROL_HPP */
