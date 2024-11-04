#pragma once

#include <string>

#include "../graph.h"

void openENWK(Graph &g, const std::string &file);
void saveENWK(const Graph &g, const std::string &filename);
