#ifndef NUMERIC_SETTING_HPP
#define NUMERIC_SETTING_HPP

#include <string>
#include <utility>
#include <memory>

#include "Optional.hpp"
#include "SettingInitializer.hpp"

class NumericSetting {
public:
    using Ptr = std::shared_ptr<NumericSetting>;

    NumericSetting(std::string name);

    virtual ~NumericSetting() = default;

    virtual std::string getName() const;

    virtual double getValue() = 0;
    virtual void setValue(double value) = 0;

private:
    std::string m_name;
};

// Числовая настройка, которая не записывается в переменные документа
class LocalNumericSetting : public NumericSetting {
public:
    LocalNumericSetting(std::string name, double value);
    LocalNumericSetting(SettingInitializer settingInitializer);

    virtual ~LocalNumericSetting() = default;

    virtual double getValue() override;
    virtual void setValue(double value) override;

private:
    double m_value;
};

// Числовая настройка со связанной переменной документа
class VariableNumericSetting : public NumericSetting {
public:
    using Range = std::pair<double, double>;

    const std::string VARIABLE_NAME_PREFIX = "kp3do_";

    VariableNumericSetting(ksPartPtr part, std::string name, double defaultValue, Range range, std::string note);
    VariableNumericSetting(ksPartPtr part, SettingInitializer settingInitializer);

    virtual ~VariableNumericSetting() = default;

    std::string getName() const override;
    double getValue() override;
    void setValue(double value) override;

private:
    ksPartPtr m_part;
    ksVariableCollectionPtr m_variableCollection;
    std::string m_note;
    double m_defaultValue;
    Range m_range;

    ksVariablePtr getVariable() const;
    ksVariablePtr getVariableOrCreateDefault();
    bool isValidValue(double value) const;
};

#endif /* NUMERIC_SETTING_HPP */
