#pragma once

#include <array>
#include <string>

#include "formatType.h"
#include "../graph.h"

#define NUM_EXTENSIONS 3

struct Format {
    FormatType type;
    std::string name;
    std::array<std::string, NUM_EXTENSIONS> exts;
    bool (*open)(Graph &, const std::string &);
    void (*save)(const Graph &, const std::string &);
};

const size_t NUM_FORMATS = static_cast<size_t>(FormatType::NUM);

extern Format formats[NUM_FORMATS];

void printFormats();
