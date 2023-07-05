#ifndef SETTING_HPP
#define SETTING_HPP

#include <string>
#include <utility>
#include <memory>
#include <comutil.h>
#include <comdef.h>

#include "SettingInitializer.hpp"

class Setting {
public:
    using Ptr = std::shared_ptr<Setting>;

    Setting(std::string name);

    virtual ~Setting() = default;

    std::string getName() const;
    virtual _variant_t getVariantValue() = 0;
    virtual void setVariantValue(_variant_t variant) = 0;

private:
    std::string m_name;
};

class StringSetting : public Setting {
public:
    using Ptr = std::shared_ptr<StringSetting>;

    StringSetting(std::string name, _bstr_t value);
    StringSetting(StringSettingInitializer settingInitializer);

    virtual ~StringSetting() = default;

    _variant_t getVariantValue() override;
    void setVariantValue(_variant_t variant) override;

    _bstr_t getValue() const;
    void setValue(_bstr_t value);

private:
    _bstr_t m_value;
};

class NumericSetting : public Setting {
public:
    using Ptr = std::shared_ptr<NumericSetting>;

    NumericSetting(std::string name);

    virtual ~NumericSetting() = default;

    virtual double getValue() = 0;
    virtual void setValue(double value) = 0;
    virtual std::string getExpression() const = 0;

};

// Числовая настройка, которая не записывается в переменные документа
class LocalNumericSetting : public NumericSetting {
public:
    LocalNumericSetting(std::string name, double value);
    LocalNumericSetting(NumericSettingInitializer settingInitializer);

    virtual ~LocalNumericSetting() = default;

    double getValue() override;
    void setValue(double value) override;
    std::string getExpression() const override;
    _variant_t getVariantValue() override;
    void setVariantValue(_variant_t variant) override;

private:
    double m_value;
};

// Числовая настройка со связанной переменной документа
class VariableNumericSetting : public NumericSetting {
public:
    using Range = std::pair<double, double>;

    const std::string VARIABLE_NAME_PREFIX = "kp3do_";

    VariableNumericSetting(ksPartPtr part, std::string name, double defaultValue, Range range, std::string note);
    VariableNumericSetting(ksPartPtr part, NumericSettingInitializer settingInitializer);

    virtual ~VariableNumericSetting() = default;

    double getValue() override;
    void setValue(double value) override;
    std::string getExpression() const override;
    _variant_t getVariantValue() override;
    void setVariantValue(_variant_t variant) override;

    std::string getVariableName() const;

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

#endif /* SETTING_HPP */
