#pragma once

#include <string>

#include "../graph.h"

bool openADMIX(Graph &g, const std::string &file);
void saveADMIX(const Graph &g, const std::string &filename);
