#include "graph.h"

Graph::~Graph() {}

void Graph::addNode(Node &n) {
    nodes.push_back(std::move(n));
    idToIndex[n.id] = nodes.size() - 1;
}

void Graph::addNode(const std::string &id, const std::string &label, std::unordered_map<std::string, std::shared_ptr<Attribute>> &attributes) {
    nodes.emplace_back(Node{id, label, std::move(attributes)});
    idToIndex[id] = nodes.size() - 1;
}

unsigned int Graph::getNumNodes() const {
    return nodes.size();
}

void Graph::addEdge(Edge &e) {
    edges[e.source].push_back(e);
}

void Graph::addEdge(const std::string &source, const std::string &target, const std::string &label, double weight, double length) {
    edges[source].emplace_back(Edge{source, target, label, weight, length});
}

unsigned int Graph::getNumEdges() const {
    unsigned int total = 0;

    for (const auto &e : edges) {
        total += e.second.size();
    }

    return total;
}
