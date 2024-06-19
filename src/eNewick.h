#pragma once

#include "graph.h"

class ENewickGraph : public Graph {
public:
    ENewickGraph();
    ENewickGraph(const Graph &other);

    ENewickGraph &operator=(const Graph &other);

    void open(const std::string &file) override;
    void save(const std::string &filename) const override;
    void print() const override;
};
