#include "graph.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
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

uint64_t Graph::getRoot() const {
    std::vector<uint64_t> inDegree(adjList.size());

    for (const auto &n : adjList) {
        for (const auto &t : n) {
            inDegree[t]++;
        }
    }

    auto it = std::find(inDegree.begin(), inDegree.end(), 0);

    if (it == inDegree.end()) {
        std::cerr << "Couldn't find root." << std::endl;
        std::exit(1);
    }

    return std::distance(inDegree.begin(), it);
}

void Graph::print() const {
    for (size_t i = 0; i < adjList.size(); i++) {
        std::cout << i << ": [";

        for (const auto &t : adjList[i]) {
            std::cout << t << ", ";
        }

        std::cout << "]" << std::endl;
    }
    std::cout << std::endl;

    std::cout << "NumNodes: " << getNumNodes() << std::endl;
    std::cout << "NumEdges: " << getNumEdges() << std::endl;
}
