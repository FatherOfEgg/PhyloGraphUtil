#include "graph.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

void Graph::addNode() {
    adjList.emplace_back();
}

unsigned int Graph::getNumNodes() const {
    return adjList.size();
}

void Graph::addEdge(const uint64_t &source, const uint64_t &target) {
    if (source >= adjList.size()) {
        addNode();
    }

    adjList[source].push_back(target);
}

unsigned int Graph::getNumEdges() const {
    unsigned int total = 0;

    for (const auto &e : adjList) {
        total += e.size();
    }

    return total;
}

void Graph::open(const std::string &file) {

}

void Graph::save(FormatType f, const std::string &filename) const {

}

void Graph::print() const {
    for (size_t i = 0; i < adjList.size(); i++) {
        std::cout << i;

        if (reticulations.find(i) != reticulations.end()) {
            std::cout << "r";
        } else {
            std::cout << " ";
        }

        std::cout << ": [";
        for (const auto &t : adjList[i]) {
            std::cout << t << ", ";
        }
        std::cout << "]" << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Reticulations: [";
    for (const auto &p : reticulations) {
        std::cout << p.first << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "Leaf nodes: [" << std::endl;
    for (const auto &p : leafName) {
        std::cout << "    " << p.first << ": " << p.second << "," << std::endl;
    }
    std::cout << "]" << std::endl;

    std::cout << std::endl;

    std::cout << "NumLeaves: " << leaves.size() << std::endl;
    std::cout << "NumReticulations: " << reticulations.size() << std::endl;
    std::cout << "NumNodes: " << getNumNodes() << std::endl;
    std::cout << "NumEdges: " << getNumEdges() << std::endl;
}
