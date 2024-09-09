#pragma once

#include "graph.h"

#include <string>

void openGML(Graph &g, const std::string &file);
void saveGML(Graph &g, const std::string &filename);
