#pragma once

#include "graph.h"

class GMLGraph : public Graph {
public:
    GMLGraph();
    GMLGraph(const Graph &other);

    GMLGraph &operator=(const Graph &other);

    void addEdge(Edge &e) override;
    void addEdge(const std::string &source, const std::string &target, const std::string &label, double weight, double length) override;

    void open(const std::string &file) override;
    void save(const std::string &filename) const override;
    void print() const override;
};
