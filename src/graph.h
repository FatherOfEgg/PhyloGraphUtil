#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "formats/formatType.h"

struct Graph {
public:
    void addNode();
    unsigned int getNumNodes() const;

    void addEdge(const uint64_t &source, const uint64_t &target);
    unsigned int getNumEdges() const;

    void open(const std::string &file);
    void save(FormatType f, const std::string &filename) const;

    void print() const;

public:
    std::vector<std::vector<uint64_t>> adjList;
    std::vector<uint64_t> leaves;
    std::unordered_map<uint64_t, std::string> leafName;
    uint64_t root;

    // The vec contains the reticulation's parents
    std::unordered_map<uint64_t, std::vector<uint64_t>> reticulations;

    FormatType format;
};
