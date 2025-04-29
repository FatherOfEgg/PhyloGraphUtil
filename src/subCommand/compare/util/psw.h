#pragma once

#include "../../../graph.h"

using PSW = std::vector<std::pair<uint64_t, uint64_t>>;

std::vector<PSW> genPSWs(const Graph &g);
