#pragma once

#include "graph.h"
#include <string>

void openENWK(Graph &g, const std::string &file);
void saveENWK(Graph &g, const std::string &filename, bool includeInternalNames);
