#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct Graph {
public:
    void addNode();
    unsigned int getNumNodes() const;

    void addEdge(const uint64_t &source, const uint64_t &target);
    unsigned int getNumEdges() const;

    uint64_t getRoot() const;

    void print() const;

public:
    std::vector<std::vector<uint64_t>> adjList;
    std::vector<uint64_t> leaves;
    std::unordered_map<uint64_t, std::string> leafName;

    std::vector<uint64_t> reticulations;
};
