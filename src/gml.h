#pragma once

#include "graph.h"

class GMLGraph : public Graph {
public:
    void open(const std::string &file) override;
    void print() const override;
};
