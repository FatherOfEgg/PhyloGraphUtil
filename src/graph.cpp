#include "graph.h"

Graph::Graph() {}

Graph::Graph(const Graph &other)
: mNodes(other.mNodes), mEdges(other.mEdges),
  mIdToIndex(other.mIdToIndex) {}

Graph::~Graph() {}

Graph &Graph::operator=(const Graph &other) {
    if (this != &other) {
        mNodes.clear();
        mEdges.clear();
        mIdToIndex.clear();

        mNodes = other.mNodes;
        mEdges = other.mEdges;
        mIdToIndex = other.mIdToIndex;
    }

    return *this;
}

void Graph::addNode(Node &n) {
    mNodes.push_back(std::move(n));
    mIdToIndex[n.id] = mNodes.size() - 1;
}

void Graph::addNode(const std::string &id, const std::string &label) {
    mNodes.emplace_back(Node{id, label});
    mIdToIndex[id] = mNodes.size() - 1;
}

unsigned int Graph::getNumNodes() const {
    return mNodes.size();
}

void Graph::addEdge(Edge &e) {
    mEdges[e.source].push_back(e);
}

void Graph::addEdge(const std::string &source, const std::string &target, const std::string &label, double length) {
    mEdges[source].emplace_back(Edge{source, target, label, length});
}

unsigned int Graph::getNumEdges() const {
    unsigned int total = 0;

    for (const auto &e : mEdges) {
        total += e.second.size();
    }

    return total;
}
