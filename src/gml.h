#pragma once

#include "graph.h"

class GMLGraph : public Graph {
public:
    GMLGraph();
    GMLGraph(const Graph &other);

    GMLGraph &operator=(const Graph &other);

    void open(const std::string &file) override;
    void save(const std::string &filename) const override;
};
