#include "Setting.hpp"

#include <string>
#include <utility>
#include <cassert>
#include <stdexcept>
#include <sstream>
#include <comdef.h>

Setting::Setting(std::string name) :
    m_name(name)
{}

std::string Setting::getName() const {
    return m_name;
}

StringSetting::StringSetting(std::string name, _bstr_t value):
    Setting(name), m_value(value)
{}

StringSetting::StringSetting(StringSettingInitializer settingInitializer):
    Setting(settingInitializer.name), m_value(settingInitializer.defaultValue)
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

NumericSetting::NumericSetting(std::string name):
    Setting(name)
{}

LocalNumericSetting::LocalNumericSetting(std::string name, double value):
    NumericSetting(name), m_value(value)
{}

LocalNumericSetting::LocalNumericSetting(NumericSettingInitializer settingInitializer):
    NumericSetting(settingInitializer.name), m_value(settingInitializer.defaultValue)
{}

double LocalNumericSetting::getValue() const {
    return m_value;
}

void LocalNumericSetting::setValue(double value) {
    m_value = value;
}

std::string LocalNumericSetting::getExpression() const {
    std::ostringstream oss;
    oss << m_value;
    return oss.str();
}

_variant_t LocalNumericSetting::getVariantValue() const {
    return _variant_t(m_value);
}

void LocalNumericSetting::setVariantValue(_variant_t variant) {
    if ((variant.vt == VT_I4) || (variant.vt == VT_R8)) {
        m_value = variant;
    }
}

VariableNumericSetting::VariableNumericSetting(ksPartPtr part, std::string name, double defaultValue, Range range, _bstr_t note):
    NumericSetting(name), m_variable(nullptr), m_range()
{
    assert(range.first < range.second);
    m_range = range;
    if (!isValidValue(defaultValue)) {
        throw std::runtime_error("Значение переменной " + getName() + " находится вне допустимого диапазона");
    }
    loadOrCreateVariable(part, name, note, defaultValue);
}

VariableNumericSetting::VariableNumericSetting(ksPartPtr part, NumericSettingInitializer settingInitializer):
    NumericSetting(settingInitializer.name), m_variable(nullptr), m_range()
{
    assert(settingInitializer.range.first < settingInitializer.range.second);
    m_range = settingInitializer.range;
    assert(isValidValue(settingInitializer.defaultValue));
    loadOrCreateVariable(part, settingInitializer.name, settingInitializer.note, settingInitializer.defaultValue);
}

double VariableNumericSetting::getValue() const {
    return m_variable->value;
}

void VariableNumericSetting::setValue(double value) {
    if (!isValidValue(value)) {
        throw std::runtime_error("Значение переменной " + getName() + " находится вне допустимого диапазона");
    }
    m_variable->value = value;
}

std::string VariableNumericSetting::getExpression() const {
    return getVariableName();
}

_variant_t VariableNumericSetting::getVariantValue() const {
    return _variant_t(getValue());
}

void VariableNumericSetting::setVariantValue(_variant_t variant) {
    if ((variant.vt == VT_I4) || (variant.vt == VT_R8)) {
        setValue(variant);
    }
}

std::string VariableNumericSetting::getVariableName() const {
    return VARIABLE_NAME_PREFIX + NumericSetting::getName();
}

void VariableNumericSetting::loadOrCreateVariable(ksPartPtr part, std::string name, _bstr_t note, double defaultValue) {
    ksFeaturePtr feature(part->GetFeature());
    ksVariableCollectionPtr variableCollection = feature->VariableCollection;
    m_variable = variableCollection->GetByName(_bstr_t(VARIABLE_NAME_PREFIX) + _bstr_t(name.c_str()), true, false);
    if (!m_variable) {
        m_variable = variableCollection->AddNewVariable(_bstr_t(VARIABLE_NAME_PREFIX) + _bstr_t(name.c_str()), defaultValue, note);
    }
}

void VariableNumericSetting::createIfNotExists(ksPartPtr part, _bstr_t note, double defaultValue) {
    _bstr_t name = m_variable->name;
    ksFeaturePtr feature(part->GetFeature());
    ksVariableCollectionPtr variableCollection = feature->VariableCollection;
    m_variable = variableCollection->GetByName(name, true, false);
    if (!m_variable) {
        m_variable = variableCollection->AddNewVariable(name, defaultValue, note);
    }
}

bool VariableNumericSetting::isValidValue(double value) const {
    if ((value < m_range.first) || (value > m_range.second)) {
        return false;
    }
    return true;
}
