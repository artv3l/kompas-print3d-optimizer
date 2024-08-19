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

}

template <kapi::ControlTypeEnum controlType>
class OptionalControl {
public:
    OptionalControl(kapi::IPropertyControlsPtr controls);

private:
    kapi::IPropertyCheckBoxPtr m_checkBox = nullptr;
    detail::GetControlType_t<controlType> m_control = nullptr;

};

template<kapi::ControlTypeEnum controlType>
inline OptionalControl<controlType>::OptionalControl(kapi::IPropertyControlsPtr controls) {
    m_checkBox = controls->Add(kapi::ControlTypeEnum::ksControlCheckBox);
    m_control = controls->Add(controlType);
}

#endif /* OPTIONAL_CONTROL_HPP */
