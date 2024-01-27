#include "Setting.hpp"

#include <string>
#include <utility>
#include <cassert>
#include <stdexcept>
#include <sstream>
#include <comdef.h>

Setting::Setting(std::string name, bool isSyncWithDocument) :
    m_name(name), m_isSyncWithDocument(isSyncWithDocument)
{}

std::string Setting::getName() const {
    return m_name;
}

bool Setting::isSyncWithDocument() const {
    return m_isSyncWithDocument;
}

StringSetting::StringSetting(std::string name, _bstr_t value):
    Setting(name, false), m_value(value)
{}

_variant_t StringSetting::getVariantValue() const {
    return _variant_t(m_value);
}

void StringSetting::setVariantValue(_variant_t variant) {
    if (variant.vt == VT_BSTR) {
        m_value = variant;
    }
}

_bstr_t StringSetting::getValue() const {
    return m_value;
}

void StringSetting::setValue(_bstr_t value) {
    m_value = value;
}

DoubleSetting::DoubleSetting(std::string name, bool isSyncWithDocument, double value):
    Setting(name, isSyncWithDocument), m_value(value)
{}

_variant_t DoubleSetting::getVariantValue() const {
    return _variant_t(m_value);
}

void DoubleSetting::setVariantValue(_variant_t variant) {
    if ((variant.vt == VT_I4) || (variant.vt == VT_R8)) {
        m_value = variant;
    }
}

double DoubleSetting::getValue() const {
    return m_value;
}

void DoubleSetting::setValue(double value) {
    m_value = value;
}

std::string DoubleSetting::getExpression() const {
    std::ostringstream oss;
    oss << m_value;
    return oss.str();
}
