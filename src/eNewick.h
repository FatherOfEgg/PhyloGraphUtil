#pragma once

#include "graph.h"

class ENewickGraph : public Graph {
public:
    void open(const std::string &file) override;
    void print() const override;
};
