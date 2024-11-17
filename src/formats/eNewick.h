#pragma once

#include <string>

#include "../graph.h"

bool openENWK(Graph &g, const std::string &file);
void saveENWK(const Graph &g, const std::string &filename);
