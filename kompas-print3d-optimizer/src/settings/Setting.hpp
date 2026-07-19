#ifndef SETTING_HPP
#define SETTING_HPP

#include <string>
#include <utility>
#include <memory>
#include <comutil.h>
#include <comdef.h>

class Setting {
public:
    using Ptr = std::shared_ptr<Setting>;

    Setting(std::string name, bool isSyncWithDocument);
    virtual ~Setting() = default;

    std::string getName() const;
    bool isSyncWithDocument() const;

    virtual _variant_t getVariantValue() const = 0;
    virtual void setVariantValue(_variant_t variant) = 0;

private:
    std::string m_name;
    bool m_isSyncWithDocument;
};

class StringSetting : public Setting {
public:
    using Ptr = std::shared_ptr<StringSetting>;

    StringSetting(std::string name, _bstr_t value);
    virtual ~StringSetting() = default;

    // Setting overrides
    virtual _variant_t getVariantValue() const override;
    virtual void setVariantValue(_variant_t variant) override;

    _bstr_t getValue() const;
    void setValue(_bstr_t value);

private:
    _bstr_t m_value;
};

class DoubleSetting : public Setting {
public:
    using Ptr = std::shared_ptr<DoubleSetting>;

    DoubleSetting(std::string name, bool isSyncWithDocument, double value);
    virtual ~DoubleSetting() = default;

    // Setting overrides
    virtual _variant_t getVariantValue() const override;
    virtual void setVariantValue(_variant_t variant) override;

    double getValue() const;
    void setValue(double value);
    std::string getExpression() const;
    std::wstring getExpressionW() const;

private:
    double m_value;
};

#endif /* SETTING_HPP */
