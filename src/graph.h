#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "formats/formats.h"

struct Graph {
public:
    void addNode();
    unsigned int getNumNodes() const;

    void addEdge(const uint64_t &source, const uint64_t &target);
    unsigned int getNumEdges() const;

    void open(const std::string &file);
    void save(Format f, const std::string &filename) const;

    void print() const;

public:
    std::vector<std::vector<uint64_t>> adjList;
    std::vector<uint64_t> leaves;
    std::unordered_map<uint64_t, std::string> leafName;

    // The vec contains the reticulation's parents
    std::unordered_map<uint64_t, std::vector<uint64_t>> reticulations;

    Format format;
};
