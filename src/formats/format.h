#pragma once

#include <string>

#include "formatType.h"
#include "../graph.h"

struct Format {
    FormatType format;
    std::string name;
    void (*open)(Graph &, const std::string &);
    void (*save)(const Graph &, const std::string &);
};

const size_t NUM_FORMATS = static_cast<size_t>(FormatType::NUM);

extern Format formats[NUM_FORMATS];

void printFormats();
