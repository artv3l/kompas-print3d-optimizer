#include "stdafx.h"
#include "settings/NumericSetting.hpp"

#include <string>
#include <utility>
#include <cassert>
#include <stdexcept>
#include "settings/NumericSetting.hpp"

NumericSetting::NumericSetting(std::string name):
    m_name(name)
{}

std::string NumericSetting::getName() const {
    return m_name;
}

LocalNumericSetting::LocalNumericSetting(std::string name, double value):
    NumericSetting(name), m_value(value)
{}

LocalNumericSetting::LocalNumericSetting(SettingInitializer settingInitializer):
    NumericSetting(settingInitializer.variableName), m_value(settingInitializer.defaultValue)
{}

double LocalNumericSetting::getValue() {
    return m_value;
}

void LocalNumericSetting::setValue(double value) {
    m_value = value;
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

VariableNumericSetting::VariableNumericSetting(ksPartPtr part, SettingInitializer settingInitializer):
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

std::string VariableNumericSetting::getName() const {
    return VARIABLE_NAME_PREFIX + NumericSetting::getName();
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

ksVariablePtr VariableNumericSetting::getVariable() const {
    return m_variableCollection->GetByName(getName().c_str(), true, false);
}

ksVariablePtr VariableNumericSetting::getVariableOrCreateDefault() {
    ksVariablePtr variable = getVariable();
    if (!variable) {
        m_variableCollection->AddNewVariable(getName().c_str(), m_defaultValue, m_note.c_str());
    }
    return variable;
}

bool VariableNumericSetting::isValidValue(double value) const {
    if ((value < m_range.first) || (value > m_range.second)) {
        return false;
    }
    return true;
}
