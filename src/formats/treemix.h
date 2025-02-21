#pragma once

#include <string>

#include "../graph.h"

bool openTreemix(Graph &g, const std::string &file);
void saveTreemix(const Graph &g, const std::string &filename);
