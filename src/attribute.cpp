#include "attribute.h"
#include <memory>
#include <string>

Attribute::~Attribute() {}

// Attribute String

AttributeString::AttributeString(const std::string &s)
: mValue(s) {}

AttributeString::AttributeString(const AttributeString &other) {
    mValue = other.mValue;
}

AttributeString &AttributeString::operator=(const AttributeString &other) {
    if (this != &other) {
        mValue = other.mValue;
    }

    return *this;
}

AttributeType AttributeString::getType() const {
    return AttributeType::STRING;
}

std::unique_ptr<Attribute> AttributeString::clone() const {
    return std::make_unique<AttributeString>(*this);
}

std::string AttributeString::getValue() const {
    return mValue;
}

void AttributeString::setValue(const std::string &s) {
    mValue = s;
}

// AttributeNumber

AttributeNumber::AttributeNumber(double n)
: mValue(n) {}

AttributeNumber::AttributeNumber(const AttributeNumber &other) {
    mValue = other.mValue;
}

AttributeNumber &AttributeNumber::operator=(const AttributeNumber &other) {
    if (this != &other) {
        mValue = other.mValue;
    }

    return *this;
}

AttributeType AttributeNumber::getType() const {
    return AttributeType::NUMBER;
}

std::unique_ptr<Attribute> AttributeNumber::clone() const {
    return std::make_unique<AttributeNumber>(*this);
}

double AttributeNumber::getValue() const {
    return mValue;
}

void AttributeNumber::setValue(double v) {
    mValue = v;
}

// Attribute Map

AttributeMap::AttributeMap() {}

AttributeMap::AttributeMap(const AttributeMap &other) {
    for (const auto &pair : other.mValues) {
        mValues[pair.first] = pair.second->clone();
    }
}

AttributeMap &AttributeMap::operator=(const AttributeMap &other) {
    if (this != &other) {
        mValues.clear();

        for (const auto &pair : other.mValues) {
            mValues[pair.first] = pair.second->clone();
        }
    }

    return *this;
}

AttributeType AttributeMap::getType() const {
    return AttributeType::LIST;
}

std::unique_ptr<Attribute> AttributeMap::clone() const {
    return std::make_unique<AttributeMap>(*this);
}

AttributeMap::Iterator AttributeMap::begin() {
    return mValues.begin();
}

AttributeMap::ConstIterator AttributeMap::begin() const {
    return mValues.begin();
}

AttributeMap::Iterator AttributeMap::end() {
    return mValues.end();
}

AttributeMap::ConstIterator AttributeMap::end() const {
    return mValues.end();
}

AttributeMap::Iterator AttributeMap::find(const std::string &key) {
    return mValues.find(key);
}

AttributeMap::ConstIterator AttributeMap::find(const std::string &key) const {
    return mValues.find(key);
}

void AttributeMap::clear() {
    mValues.clear();
}

void AttributeMap::add(const std::string &s, std::unique_ptr<Attribute> value) {
    mValues[s] = std::move(value);
}

AttributeMap *AttributeMap::getMap(const std::string &key) {
    return dynamic_cast<AttributeMap *>(mValues[key].get());
}

std::string AttributeMap::getStringValue(const std::string &key) const {
    return dynamic_cast<AttributeString *>(mValues.at(key).get())->getValue();
}

double AttributeMap::getNumberValue(const std::string &key) const {
    return dynamic_cast<AttributeNumber *>(mValues.at(key).get())->getValue();
}
