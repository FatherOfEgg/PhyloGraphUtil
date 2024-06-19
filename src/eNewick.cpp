#include "eNewick.h"
#include "graph.h"

#include <iostream>
#include <string>

ENewickGraph::ENewickGraph() {}

ENewickGraph::ENewickGraph(const Graph &other)
: Graph(other) {}

ENewickGraph &ENewickGraph::operator=(const Graph &other) {
    if (this != &other) {
        Graph::operator=(other);
    }

    return *this;
}

void ENewickGraph::open(const std::string &file) {

}

void ENewickGraph::save(const std::string &filename) const {
    std::cout << "Save as ENewick" << std::endl;
}

void ENewickGraph::print() const {

}
