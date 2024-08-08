#include "graph.h"

#include <iostream>

void Graph::addNode(Node &n) {
    nodes.push_back(std::move(n));
    idToIndex[n.id] = nodes.size() - 1;
}

void Graph::addNode(const std::string &id, const std::string &label) {
    nodes.emplace_back(Node{id, label});
    idToIndex[id] = nodes.size() - 1;
}

unsigned int Graph::getNumNodes() const {
    return nodes.size();
}

void Graph::addEdge(Edge &e) {
    edges[e.source].push_back(e);
}

void Graph::addEdge(const std::string &source, const std::string &target, const std::string &label, double length) {
    edges[source].emplace_back(Edge{source, target, label, length});
}

unsigned int Graph::getNumEdges() const {
    unsigned int total = 0;

    for (const auto &e : edges) {
        total += e.second.size();
    }

    return total;
}

void Graph::print() const {
    std::cout << "Nodes:" << std::endl;
    for (const auto &n : nodes) {
        std::cout << n.id << ", \"" << n.label << "\"" << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Edges:" << std::endl;
    for (const auto &e : edges) {
        std::cout << e.first << ": ";

        for (const auto &x : e.second) {
            std::cout << x.target << ", ";
        }

        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "NumNodes: " << getNumNodes() << std::endl;
    std::cout << "NumEdges: " << getNumEdges() << std::endl;
}
