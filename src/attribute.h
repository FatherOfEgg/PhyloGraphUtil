#pragma once

#include <memory>
#include <string>
#include <unordered_map>

enum class AttributeType {
    LIST,
    STRING,
    NUMBER,
};

class Attribute {
public:
    virtual ~Attribute();

    virtual AttributeType getType() const = 0;
    virtual std::unique_ptr<Attribute> clone() const = 0;
};

class AttributeString : public Attribute {
public:
    AttributeString(const std::string &s);
    AttributeString(const AttributeString &other);

    AttributeString &operator=(const AttributeString &other);

    AttributeType getType() const override;
    std::unique_ptr<Attribute> clone() const override;

    std::string getValue() const;
    void setValue(const std::string &s);

private:
    std::string mValue;
};

class AttributeNumber : public Attribute {
public:
    AttributeNumber(double n);
    AttributeNumber(const AttributeNumber &other);

    AttributeNumber &operator=(const AttributeNumber &other);

    AttributeType getType() const override;
    std::unique_ptr<Attribute> clone() const override;

    double getValue() const;
    void setValue(double v);

private:
    double mValue;
};

class AttributeMap : public Attribute {
public:
    AttributeMap();
    AttributeMap(const AttributeMap &other);

    AttributeMap &operator=(const AttributeMap &other);

    AttributeType getType() const override;
    std::unique_ptr<Attribute> clone() const override;

    using Iterator = std::unordered_map<std::string, std::unique_ptr<Attribute>>::iterator;
    using ConstIterator = std::unordered_map<std::string, std::unique_ptr<Attribute>>::const_iterator;

    Iterator begin();
    ConstIterator begin() const;

    Iterator end();
    ConstIterator end() const;

    Iterator find(const std::string &key);
    ConstIterator find(const std::string &key) const;

    void clear();
    void add(const std::string &s, std::unique_ptr<Attribute> value);

    AttributeMap *getMap(const std::string &key);

    std::string getStringValue(const std::string &key) const;
    double getNumberValue(const std::string &key) const;

private:
    std::unordered_map<std::string, std::unique_ptr<Attribute>> mValues;
};
