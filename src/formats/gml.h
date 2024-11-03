#pragma once

#include <string>

#include "../graph.h"

void openGML(Graph &g, const std::string &file);
void saveGML(Graph &g, const std::string &filename);
