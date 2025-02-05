#pragma once

#include "bitmask.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../../../graph.h"

using BitmaskSet = std::unordered_set<Bitmask, BitmaskHash>;

std::vector<BitmaskSet> computeClusters(
    const Graph &g,
    const std::unordered_map<std::string, uint64_t> &bitmaskId
);
