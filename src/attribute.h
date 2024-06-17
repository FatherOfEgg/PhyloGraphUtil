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
};

class AttributeList : public Attribute {
public:
    AttributeType getType() const override;

    void add(const std::string &s, std::shared_ptr<Attribute> value);

public:
    std::unordered_map<std::string, std::shared_ptr<Attribute>> values;
};

class AttributeString : public Attribute {
public:
    AttributeString(const std::string &s);

    AttributeType getType() const override;

public:
    std::string value;
};

class AttributeNumber : public Attribute {
public:
    AttributeNumber(double n);

    AttributeType getType() const override;

public:
    double value;
};
