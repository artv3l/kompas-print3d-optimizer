#include "stdafx.h"
#include "settings/Setting.hpp"

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
    Setting(settingInitializer.variableName), m_value(settingInitializer.defaultValue)
{}

_variant_t StringSetting::getVariantValue() {
    return _variant_t(m_value);
}

void StringSetting::setVariantValue(_variant_t variant) {
    if (variant.vt == VT_BSTR) {
        m_value = variant;
    }
}

std::string StringSetting::getExpression() const {
    return std::string(m_value);
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
    NumericSetting(settingInitializer.variableName), m_value(settingInitializer.defaultValue)
{}

double LocalNumericSetting::getValue() {
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

_variant_t LocalNumericSetting::getVariantValue() {
    return _variant_t(m_value);
}

void LocalNumericSetting::setVariantValue(_variant_t variant) {
    if ((variant.vt == VT_I4) || (variant.vt == VT_R8)) {
        m_value = variant;
    }
}

VariableNumericSetting::VariableNumericSetting(ksPartPtr part, std::string name, double defaultValue, Range range, std::string note):
    NumericSetting(name), m_part(part), m_variableCollection(nullptr), m_note(note), m_defaultValue(0.0), m_range()
{
    assert(range.first < range.second);
    if (!isValidValue(defaultValue)) {
        throw std::runtime_error("Значение переменной " + getName() + " находится вне допустимого диапазона");
    }
    m_range = range;
    m_defaultValue = defaultValue;

    ksFeaturePtr feature(part->GetFeature());
    m_variableCollection = feature->VariableCollection;

    getVariableOrCreateDefault(); // создаем переменную
}

VariableNumericSetting::VariableNumericSetting(ksPartPtr part, NumericSettingInitializer settingInitializer):
    NumericSetting(settingInitializer.variableName), m_part(part), m_variableCollection(nullptr), m_note(settingInitializer.variableNote), m_defaultValue(0.0), m_range()
{
    assert(settingInitializer.range.first < settingInitializer.range.second);
    m_range = settingInitializer.range;
    assert(isValidValue(settingInitializer.defaultValue));
    m_defaultValue = settingInitializer.defaultValue;

    ksFeaturePtr feature(part->GetFeature());
    m_variableCollection = feature->VariableCollection;

    getVariableOrCreateDefault(); // создаем переменную
}

double VariableNumericSetting::getValue() {
    ksVariablePtr variable = getVariableOrCreateDefault();
    return variable->value;
}

void VariableNumericSetting::setValue(double value) {
    if (!isValidValue(value)) {
        throw std::runtime_error("Значение переменной " + getName() + " находится вне допустимого диапазона");
    }
    ksVariablePtr variable = getVariableOrCreateDefault();
    variable->value = value;
}

std::string VariableNumericSetting::getExpression() const {
    return getVariableName();
}

_variant_t VariableNumericSetting::getVariantValue() {
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

ksVariablePtr VariableNumericSetting::getVariable() const {
    return m_variableCollection->GetByName(getVariableName().c_str(), true, false);
}

ksVariablePtr VariableNumericSetting::getVariableOrCreateDefault() {
    ksVariablePtr variable = getVariable();
    if (!variable) {
        m_variableCollection->AddNewVariable(getVariableName().c_str(), m_defaultValue, m_note.c_str());
    }
    return variable;
}

bool VariableNumericSetting::isValidValue(double value) const {
    if ((value < m_range.first) || (value > m_range.second)) {
        return false;
    }
    return true;
}

