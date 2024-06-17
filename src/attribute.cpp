#include "attribute.h"

Attribute::~Attribute() {}

AttributeType AttributeList::getType() const {
    return AttributeType::LIST;
}

void AttributeList::add(const std::string &s, std::shared_ptr<Attribute> value) {
    values[s] = std::move(value);
}

AttributeString::AttributeString(const std::string &s)
: value(s) {}

AttributeType AttributeString::getType() const {
    return AttributeType::STRING;
}

AttributeNumber::AttributeNumber(double n)
: value(n) {}

AttributeType AttributeNumber::getType() const {
    return AttributeType::NUMBER;
}
