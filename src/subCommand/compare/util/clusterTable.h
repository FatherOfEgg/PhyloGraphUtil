#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../../../graph.h"
#include "psw.h"

using Clusters = std::vector<std::pair<uint64_t, uint64_t>>;

struct LRNW {
    uint64_t L, R, N, W;
};

struct ClusterTable {
public:
    ClusterTable(const Graph &g, const PSW &psw);

    uint64_t encode(const std::string &l) const;

    bool isClust(uint64_t L, uint64_t R) const;

public:
    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> ct;
    std::unordered_map<std::string, uint64_t> internalLabels;
};
