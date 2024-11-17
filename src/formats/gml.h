#pragma once

#include <string>

#include "../graph.h"

bool openGML(Graph &g, const std::string &file);
void saveGML(const Graph &g, const std::string &filename);
